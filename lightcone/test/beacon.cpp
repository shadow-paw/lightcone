#include <stdio.h>
#include "lightcone/lightcone.h"

using lightcone::Beacon;
using lightcone::SockAddr;
using lightcone::Threads;
using lightcone::Network;

// -----------------------------------------------------------
class BeaconServer : public Beacon {
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
    bool cb_beacon_peerup(uint32_t type, uint32_t id, const SockAddr& addr) {
        peerup = true;
        return true;
    }
    bool cb_beacon_peerdown(uint32_t type, uint32_t id, const SockAddr& addr) {
        peerdown = true;
        return true;
    }
};
// -----------------------------------------------------------
bool test_discovery() {
    bool result = false;
    BeaconServer alice, bob;
    const char mcast_addr[] = "239.0.0.4:8888";
    if (!alice.init(mcast_addr, 1234, 1, 6000, 100, 5000)) return false;
    if (!bob.init(mcast_addr, 1234, 1, 6001, 100, 5000)) return false;
    alice.start();
    bob.start();
    for (int i=0; i < 100*10; i++) {
        if (alice.ready && alice.peerup && bob.ready && bob.peerup) {
            result = true;
            break;
        }
        Threads::msleep(10);
    }
    bob.stop();
    alice.stop();
    return result;
}
// -----------------------------------------------------------
bool run_tests() {
    if (!test_discovery()) { printf ("FAILED. test_discovery()\n"); return false; }
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
