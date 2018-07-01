#include <stdio.h>
#include "lightcone/lightcone.h"

// -----------------------------------------------------------
bool udp_pingpong() {
    lightcone::Udp udp1, udp2;
    lightcone::SockAddr bindaddr;
    if (!udp1.open(true) || !udp2.open(true)) return false;
    if (!bindaddr.ip4("0.0.0.0", 8888)) return false;
    if (!udp1.bind(bindaddr, true)) return false;
    for (int i=0; i < 10; i++) {
        const char payload[] = "HELLO";
        udp2.send(bindaddr, payload, sizeof(payload));
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
    lightcone::SockAddr addr;
    if (!addr.ip4("239.0.0.4", 8888)) return false;
    if (!udp1.open(true) || !udp2.open(true)) return false;
    if (!udp1.bind(addr, true)) return false;
    if (!udp1.joinmcast(addr)) return false;
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
bool run_tests() {
    if (!udp_pingpong()) { printf ("FAILED. udp_pingpong()\n"); return false; }
    if (!udp_mcast()) { printf ("FAILED. udp_mcast()\n"); return false; }
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
