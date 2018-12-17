#include <stdio.h>
#include "lightcone/lightcone.h"
#include "unittest.h"

// -----------------------------------------------------------
bool udp_pingpong() {
    lightcone::Udp udp1, udp2;
    lightcone::SockAddr addr;
    if (!addr.set_ip4("0.0.0.0", 8888)) return false;
    if (!udp1.open(addr.get_domain(), true) || !udp2.open(addr.get_domain(), true)) return false;
    if (!udp1.bind(addr, true)) return false;
    for (int i=0; i < 10; i++) {
        const char payload[] = "HELLO";
        udp2.send(addr, payload, sizeof(payload));
        char rbuf[8192];
        lightcone::SockAddr sender;
        ssize_t rlen = udp1.recv(&sender, rbuf, sizeof(rbuf), 1000);
        if (rlen > 0) return true;
    }
    return false;
}
// -----------------------------------------------------------
bool udp_mcast() {
    lightcone::Udp udp1, udp2;
    lightcone::SockAddr addr1, addr2;
    if (!addr1.set_ip4("0.0.0.0", 8888)) return false;
    if (!addr2.set_ip4("239.0.0.4", 8888)) return false;
    if (!udp1.open(addr1.get_domain(), true)) return false;
    if (!udp2.open(addr2.get_domain(), true)) return false;
    if (!udp1.bind(addr1, true)) return false;
    if (!udp1.joinmcast(addr2)) return false;
    if (!udp2.joinmcast(addr2)) return false;
    for (int i=0; i < 10; i++) {
        const char payload[] = "HELLO";
        udp2.send(addr2, payload, sizeof(payload));
        char rbuf[8192];
        lightcone::SockAddr sender;
        ssize_t rlen = udp1.recv(&sender, rbuf, sizeof(rbuf), 1000);
        if (rlen > 0) return true;
    }
    return false;
}
// -----------------------------------------------------------
bool udp_mcast6() {
    lightcone::Udp udp1, udp2;
    lightcone::SockAddr addr1;
    lightcone::SockAddr addr2;
    if (!addr1.set_ip6("::", 8888)) return false;
    if (!addr2.set_ip6("ff05:0:0:0:0:0:0:2", 8888)) return false;
    if (!udp1.open(addr1.get_domain(), true)) return false;
    if (!udp2.open(addr2.get_domain(), true)) return false;
    if (!udp1.bind(addr1, true)) return false;
    if (!udp1.joinmcast(addr2)) return false;
    if (!udp2.joinmcast(addr2)) return false;
    for (int i=0; i < 10; i++) {
        const char payload[] = "HELLO";
        udp2.send(addr2, payload, sizeof(payload));
        char rbuf[8192];
        lightcone::SockAddr sender;
        ssize_t rlen = udp1.recv(&sender, rbuf, sizeof(rbuf), 1000);
        if (rlen > 0) return true;
    }
    return false;
}
// -----------------------------------------------------------
int main(int argc, char* argv[]) {
    lightcone::Network::start();
    UnitTest t(__FILE__);
    t.run("udp_pingpong", udp_pingpong);
    t.run("udp_mcast", udp_mcast);
    // NOTE: travis-ci don't have ipv6 multicast
    // r.run("udp_mcast6", udp_mcast6);
    lightcone::Network::stop();
    return t.failed ? 1 : 0;
}
// -----------------------------------------------------------
