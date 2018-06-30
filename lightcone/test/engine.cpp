#include <stdio.h>
#include "lightcone/lightcone.h"

using lightcone::Network;
using lightcone::NetEngine;
using lightcone::SockAddr;
using lightcone::Tcp;
using lightcone::Threads;
using lightcone::LoadBalancerRR;

// -----------------------------------------------------------
class TestServer : public NetEngine {
 public:
    bool accepted, opened, closed, sent, recv, timeout;

    TestServer() : NetEngine(&m_lb) {
        accepted = opened = closed = sent = recv = false;
        set_timeout(1000);
    }
    ~TestServer() = default;

 protected:
    bool cb_net_timeout(Tcp* conn, uint64_t now) {
        timeout = true;
        return false;
    }
    bool cb_net_refused(Tcp* conn, uint64_t now) {
        return true;
    }
    bool cb_net_accepted(Tcp* conn, uint64_t now) {
        accepted = true;
        conn->ud.u32_1 = 1;
        return true;
    }
    bool cb_net_opened(Tcp* conn, uint64_t now) {
        opened = true;
        conn->send("hello");
        return true;
    }
    bool cb_net_closed(Tcp* conn, uint64_t now) {
        closed = true;
        return true;
    }
    bool cb_net_sent(Tcp* conn, uint64_t now) {
        sent = true;
        return true;
    }
    bool cb_net_recv(Tcp* conn, uint64_t now) {
        recv = true;
        conn->recv([this, conn](const uint8_t* rbuf, size_t rlen) -> size_t {
            if (conn->ud.u32_1) {
                conn->send(rbuf, rlen);
            }
            return rlen;
        });
        return true;
    }

 private:
    LoadBalancerRR<uint32_t> m_lb;
};
// -----------------------------------------------------------
bool engine_pingpong() {
    bool result = false;
    TestServer s;
    SockAddr addr;
    if (!s.start(2)) return false;
    if (!s.listen(SockAddr("0.0.0.0", 8888))) return false;
    if (!s.connect(SockAddr("127.0.0.1", 8888))) return false;

    for (int i=0; i < 100*5; i++) {
        if (s.accepted && s.opened && s.closed && s.sent && s.recv && s.timeout) {
            result = true;
            break;
        }
        Threads::msleep(10);
    }

    s.stop();
    return result;
}
// -----------------------------------------------------------
bool run_tests() {
    if (!engine_pingpong()) { printf ("FAILED. engine_pingpong()\n"); return false; }
    return true;
}
// -----------------------------------------------------------
int main(int argc, char* argv[]) {
    Network::start();
    bool success = run_tests();
    Network::stop();
    return success ? 0 : 1;
}
// -----------------------------------------------------------
