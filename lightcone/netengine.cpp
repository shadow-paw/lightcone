#include <list>
#include "calendar.h"
#include "netengine.h"
#include "netpoller.h"
#include "sysinfo.h"

namespace lightcone {
// -----------------------------------------------------------
NetEngine::NetEngine(LoadBalancer<uint32_t>* lb) {
    _lb = lb;
    _timeout = 0;
}
void NetEngine::set_timeout(uint64_t timeout) {
    _timeout = timeout;
}
bool NetEngine::start(int nu_worker) {
    if (nu_worker < 0) {
        nu_worker = SysInfo::cpu_core() * (-nu_worker);
    }
    if (nu_worker <= 0 || nu_worker > kMaxWorker) return false;
    _lb->setup(1, nu_worker);
    return Threads::start((unsigned int)(nu_worker+1));  // +1 for listen thread
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
    conn->_managed = true;
    for (;;) {
        if (_inbox[0].post(conn)) return true;
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
    conn->_managed = true;
    conn->_lb_hash = addr.hash();
    int lb = _lb->retain(conn->_lb_hash);
    for (;;) {
        if (_inbox[lb].post(conn)) return true;
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
            while (_inbox[id].get(&conn, 0)) {
                sockets.push_back(conn);
                poller.add(conn->_socket, NetPoller::kRead, conn);
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
            auto now = Calendar::now();
            while ((accepted = listener->accept(&remote))!= nullptr) {
                accepted->set_nonblocking(true);
                accepted->_managed = true;
                accepted->_lb_hash = remote.hash();
                accepted->_state = Tcp::State::Connected;

                if (!cb_net_accepted(accepted, listener, now)) {
                    accepted->close();
                } else {
                    accepted->_atime = now;
                }
                int lb = _lb->retain(accepted->_lb_hash);
                while (!_inbox[lb].post(accepted)) {
                    msleep(1);
                }
            }
            return true;
        });
    }
    {
        Tcp* conn;
        // Kill unprocessed sockets
        while (_inbox[id].get(&conn, 0)) {
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
            while (_inbox[id].get(&newconn, 0)) {
                sockets.push_back(newconn);
                if (newconn->_state == Tcp::State::Connnecting) {
                    newconn->_evmask = NetPoller::kWrite;
                    poller.add(newconn->_socket, NetPoller::kReadWrite, newconn);
                } else {
                    poller.add(newconn->_socket, NetPoller::kRead, newconn);
                }
            }
        }
        // Don't drain CPU if no socket
        if (sockets.empty()) {
            msleep(10);
            continue;
        }

        auto now = Calendar::now();
        auto timeout = _timeout;

        // Walk socket list
        for (auto it=sockets.begin(); it != sockets.end(); ) {
            Tcp* conn = *it;
            if (conn->_state == Tcp::State::Closing) {
                conn->_state = Tcp::State::Closed;
                cb_net_closed(conn, now);
                _lb->release(conn->_lb_hash);
                poller.remove(conn->_socket);
                delete conn;
                it = sockets.erase(it);
                continue;
            } else if (conn->_state == Tcp::State::Refused) {
                conn->_state = Tcp::State::Closed;
                cb_net_refused(conn, now);
                _lb->release(conn->_lb_hash);
                poller.remove(conn->_socket);
                delete conn;
                it = sockets.erase(it);
                continue;
            } else if (conn->_state == Tcp::State::Connnecting) {
                if ((conn->_evmask & NetPoller::kWrite) == 0) {
                    conn->_evmask |= NetPoller::kWrite;
                    poller.modify(conn->_socket, NetPoller::kReadWrite, conn);
                }
            }
            it++;

            if (conn->_state == Tcp::State::Connected) {
                // Check for timeout
                if (timeout != 0 && now - conn->_atime >= timeout) {
                    if (!cb_net_timeout(conn, now)) {
                        conn->close();
                        continue;
                    }
                    conn->_atime = now;
                }
                if (conn->_obuf.size() > 0) {
                    if ((conn->_evmask & NetPoller::kWrite) == 0) {
                        conn->_evmask |= NetPoller::kWrite;
                        poller.modify(conn->_socket, NetPoller::kReadWrite, conn);
                    }
                } else {
                    if ((conn->_evmask & NetPoller::kWrite) != 0) {
                        conn->_evmask ^= NetPoller::kWrite;
                        poller.modify(conn->_socket, NetPoller::kRead, conn);
                    }
                }
            }
        }
        // Wait for events
        poller.poll(100, [this, now](const RAW_SOCKET& fd, int event, void* ud) -> bool {
            Tcp* conn = static_cast<Tcp*>(ud);
            if (event & NetPoller::kWrite) {
                conn->_atime = now;
                if (conn->_state == Tcp::State::Connnecting) {
                    conn->_state = Tcp::State::Connected;
                    if (!cb_net_opened(conn, now)) {
                        conn->close();
                        return true;
                    }
                }
                conn->io_write();
                if (conn->_obuf.size() ==0) {
                    if (!cb_net_sent(conn, now)) {
                        conn->close();
                        return true;
                    }
                }
            }
            if (event & NetPoller::kRead) {
                conn->_atime = now;
                conn->io_read();
                if (conn->_ibuf.size() > 0) {
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
        while (_inbox[id].get(&conn, 0)) {
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
