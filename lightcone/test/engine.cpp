#include <stdio.h>
#include "lightcone/lightcone.h"

// -----------------------------------------------------------
class TestServer : public lightcone::NetEngine {
 public:
    bool accepted, opened, closed, sent, recv, timeout;

    TestServer() : lightcone::NetEngine(&m_lb) {
        accepted = opened = closed = sent = recv = false;
        set_timeout(1000);
    }
    ~TestServer() = default;

 protected:
    bool cb_net_timeout(lightcone::Tcp* conn, uint64_t now) {
        timeout = true;
        return false;
    }
    bool cb_net_refused(lightcone::Tcp* conn, uint64_t now) {
        return true;
    }
    bool cb_net_accepted(lightcone::Tcp* conn, uint64_t now) {
        accepted = true;
        conn->ud.u32_1 = 1;
        return true;
    }
    bool cb_net_opened(lightcone::Tcp* conn, uint64_t now) {
        opened = true;
        conn->send("hello");
        return true;
    }
    bool cb_net_closed(lightcone::Tcp* conn, uint64_t now) {
        closed = true;
        return true;
    }
    bool cb_net_sent(lightcone::Tcp* conn, uint64_t now) {
        sent = true;
        return true;
    }
    bool cb_net_recv(lightcone::Tcp* conn, uint64_t now) {
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
    lightcone::LoadBalancerRR<uint32_t> m_lb;
};
// -----------------------------------------------------------
bool engine_pingpong() {
    bool result = false;
    TestServer s;
    lightcone::SockAddr addr;
    if (!s.start(2)) return false;
    if (!s.listen(lightcone::SockAddr("0.0.0.0", 8888))) return false;
    if (!s.connect(lightcone::SockAddr("127.0.0.1", 8888))) return false;

    for (int i=0; i < 100*5; i++) {
        if (s.accepted && s.opened && s.closed && s.sent && s.recv && s.timeout) {
            result = true;
            break;
        }
        lightcone::Threads::msleep(10);
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
    lightcone::Network::start();
    bool success = run_tests();
    lightcone::Network::stop();
    return success ? 0 : 1;
}
// -----------------------------------------------------------
