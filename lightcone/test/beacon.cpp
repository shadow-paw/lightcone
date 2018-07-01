#include <stdio.h>
#include "lightcone/lightcone.h"

// -----------------------------------------------------------
class BeaconServer : public lightcone::Beacon {
 public:
    bool ready, peerup, peerdown;
    BeaconServer() {
        ready = peerup = peerdown = false;
    }

 protected:
    uint32_t cb_beacon_createid(uint32_t type, uint32_t index) {
        return ((0xff & type) << 8) | (0xff & index);
    }
    bool cb_beacon_ready(uint32_t type, uint32_t id) {
        ready = true;
        return true;
    }
    bool cb_beacon_peerup(uint32_t type, uint32_t id, const lightcone::SockAddr& addr) {
        peerup = true;
        return true;
    }
    bool cb_beacon_peerdown(uint32_t type, uint32_t id, const lightcone::SockAddr& addr) {
        peerdown = true;
        return true;
    }
};
// -----------------------------------------------------------
bool test_discovery() {
    bool result = false;
    BeaconServer alice, bob;
    const char mcast_addr[] = "239.0.0.4";
    if (!alice.init(mcast_addr, 8888, 1234, 1, 6000, 100, 5000)) return false;
    if (!bob.init(mcast_addr, 8888, 1234, 1, 6001, 100, 5000)) return false;
    alice.start();
    bob.start();
    for (int i=0; i < 100*10; i++) {
        if (alice.ready && alice.peerup && bob.ready && bob.peerup) {
            result = true;
            break;
        }
        lightcone::Threads::msleep(10);
    }
    bob.stop();
    alice.stop();
    return result;
}
// -----------------------------------------------------------
bool test_discovery6() {
    bool result = false;
    BeaconServer alice, bob;
    const char mcast_addr[] = "FF05:0:0:0:0:0:0:2";
    if (!alice.init(mcast_addr, 8888, 1234, 1, 7000, 100, 5000)) return false;
    if (!bob.init(mcast_addr, 8888, 1234, 1, 7001, 100, 5000)) return false;
    alice.start();
    bob.start();
    for (int i=0; i < 100*10; i++) {
        if (alice.ready && alice.peerup && bob.ready && bob.peerup) {
            result = true;
            break;
        }
        lightcone::Threads::msleep(10);
    }
    bob.stop();
    alice.stop();
    return result;
}
// -----------------------------------------------------------
bool run_tests() {
    if (!test_discovery()) { printf ("FAILED. test_discovery()\n"); return false; }
    // NOTE: travis-ci don't have ipv6 multicast
    // if (!test_discovery6()) { printf ("FAILED. test_discovery6()\n"); return false; }
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
