#include <list>
#include "calendar.h"
#include "netengine.h"
#include "netpoller.h"
#include "sysinfo.h"

namespace lightcone {
// -----------------------------------------------------------
NetEngine::NetEngine(LoadBalancer<uint32_t>* lb) {
    m_lb = lb;
    m_timeout = 0;
}
void NetEngine::set_timeout(uint64_t timeout) {
    m_timeout = timeout;
}
bool NetEngine::start(int num_worker) {
    if (num_worker < 0) {
        num_worker = SysInfo::cpu_core() * (-num_worker);
    }
    if (num_worker <= 0 || num_worker > kMaxWorker) return false;
    m_lb->setup(1, num_worker);
    return Threads::start((unsigned int)(num_worker+1));  // +1 for listen thread
}
void NetEngine::stop() {
    Threads::stop();
}
bool NetEngine::listen(const SockAddr& addr, std::function<bool(Tcp*)> initializer) {
    Tcp* conn = new Tcp();
    if (conn == nullptr) return false;
    if (initializer && !initializer(conn)) {
        delete conn;
        return false;
    }
    if (!conn->listen(addr, true)) {
        delete conn;
        return false;
    }
    conn->m_managed = true;
    for (;;) {
        if (m_inbox[0].post(conn)) return true;
        msleep(1);
    }
}
bool NetEngine::connect(const SockAddr& addr, std::function<bool(Tcp*)> initializer) {
    Tcp* conn = new Tcp();
    if (conn == nullptr) return false;
    if (initializer && !initializer(conn)) {
        delete conn;
        return false;
    }
    if (!conn->connect(addr, true)) {
        delete conn;
        return false;
    }
    conn->m_managed = true;
    conn->m_lb_hash = addr.hash();
    int lb = m_lb->retain(conn->m_lb_hash);
    for (;;) {
        if (m_inbox[lb].post(conn)) return true;
        msleep(1);
    }
}
void NetEngine::worker(unsigned int id, bool* runflag) {
    if (id == 0) {
        worker_listen(id, runflag);
    } else {
        worker_socket(id, runflag);
    }
}
void NetEngine::worker_listen(unsigned int id, bool* runflag) {
    NetPoller poller;
    std::list<Tcp*> sockets;
    if (!poller.init()) return;

    while (*runflag) {
        // Take sockets from inbox
        {
            Tcp* conn;
            while (m_inbox[id].get(&conn, 0)) {
                sockets.push_back(conn);
                poller.add(conn->m_socket, NetPoller::kRead, conn);
            }
        }
        // Don't drain CPU if no socket
        if (sockets.empty()) {
            msleep(10);
            continue;
        }
        // Poll for event, accept socket and dispatch to worker.
        poller.poll(100, [this](const RAW_SOCKET& fd, int ev, void* ud) -> bool {
            Tcp* listener = static_cast<Tcp*>(ud);
            Tcp* accepted;
            SockAddr remote;
            while ((accepted = listener->accept(&remote))!= nullptr) {
                accepted->set_nonblocking(true);
                accepted->m_managed = true;
                accepted->m_lb_hash = remote.hash();
                int lb = m_lb->retain(accepted->m_lb_hash);
                while (!m_inbox[lb].post(accepted)) {
                    msleep(1);
                }
            }
            return true;
        });
    }
    {
        Tcp* conn;
        // Kill unprocessed sockets
        while (m_inbox[id].get(&conn, 0)) {
            delete conn;
        }
        // Kill managed sockets
        for (auto it=sockets.begin(); it != sockets.end(); ++it) {
            delete *it;
        }
    }
}
void NetEngine::worker_socket(unsigned int id, bool* runflag) {
    NetPoller poller;
    std::list<Tcp*> sockets;
    if (!poller.init()) return;

    while (*runflag) {
        // Take sockets from inbox
        {
            Tcp* newconn = nullptr;
            while (m_inbox[id].get(&newconn, 0)) {
                sockets.push_back(newconn);
                if (newconn->m_state == Tcp::State::Connnecting) {
                    newconn->m_evmask = NetPoller::kWrite;
                    poller.add(newconn->m_socket, NetPoller::kReadWrite, newconn);
                } else {
                    poller.add(newconn->m_socket, NetPoller::kRead, newconn);
                }
            }
        }
        // Don't drain CPU if no socket
        if (sockets.empty()) {
            msleep(10);
            continue;
        }

        auto now = Calendar::now();
        auto timeout = m_timeout;

        // Walk socket list
        for (auto it=sockets.begin(); it != sockets.end(); ) {
            Tcp* conn = *it;
            if (conn->m_state == Tcp::State::Closing) {
                conn->m_state = Tcp::State::Closed;
                cb_net_closed(conn, now);
                m_lb->release(conn->m_lb_hash);
                poller.remove(conn->m_socket);
                delete conn;
                it = sockets.erase(it);
                continue;
            } else if (conn->m_state == Tcp::State::Refused) {
                conn->m_state = Tcp::State::Closed;
                cb_net_refused(conn, now);
                m_lb->release(conn->m_lb_hash);
                poller.remove(conn->m_socket);
                delete conn;
                it = sockets.erase(it);
                continue;
            } else if (conn->m_state == Tcp::State::Accepted) {
                conn->m_state = Tcp::State::Connected;
                if (!cb_net_accepted(conn, now)) {
                    conn->close();
                } else {
                    conn->m_atime = now;
                }
            } else if (conn->m_state == Tcp::State::Connnecting) {
                if ((conn->m_evmask & NetPoller::kWrite) == 0) {
                    conn->m_evmask |= NetPoller::kWrite;
                    poller.modify(conn->m_socket, NetPoller::kReadWrite, conn);
                }
            }
            it++;

            if (conn->m_state == Tcp::State::Connected) {
                // Check for timeout
                if (timeout != 0 && now - conn->m_atime >= timeout) {
                    if (!cb_net_timeout(conn, now)) {
                        conn->close();
                        continue;
                    }
                    conn->m_atime = now;
                }
                if (conn->m_obuf.size() > 0) {
                    if ((conn->m_evmask & NetPoller::kWrite) == 0) {
                        conn->m_evmask |= NetPoller::kWrite;
                        poller.modify(conn->m_socket, NetPoller::kReadWrite, conn);
                    }
                } else {
                    if ((conn->m_evmask & NetPoller::kWrite) != 0) {
                        conn->m_evmask ^= NetPoller::kWrite;
                        poller.modify(conn->m_socket, NetPoller::kRead, conn);
                    }
                }
            }
        }
        // Wait for events
        poller.poll(100, [this, now](const RAW_SOCKET& fd, int event, void* ud) -> bool {
            Tcp* conn = static_cast<Tcp*>(ud);
            if (event & NetPoller::kWrite) {
                conn->m_atime = now;
                if (conn->m_state == Tcp::State::Connnecting) {
                    conn->m_state = Tcp::State::Connected;
                    if (!cb_net_opened(conn, now)) {
                        conn->close();
                        return true;
                    }
                }
                conn->io_write();
                if (conn->m_obuf.size() ==0) {
                    if (!cb_net_sent(conn, now)) {
                        conn->close();
                        return true;
                    }
                }
            }
            if (event & NetPoller::kRead) {
                conn->m_atime = now;
                conn->io_read();
                if (conn->m_ibuf.size() > 0) {
                    if (!cb_net_recv(conn, now)) {
                        conn->close();
                        return true;
                    }
                }
            }
            return true;
        });
    }  // runflag

    {
        Tcp* conn;
        // Kill unprocessed sockets
        while (m_inbox[id].get(&conn, 0)) {
            delete conn;
        }
        // Kill managed sockets
        for (auto it=sockets.begin(); it != sockets.end(); ++it) {
            delete *it;
        }
    }
}
// -----------------------------------------------------------
}  // namespace lightcone
