#include <stdio.h>
#include "lightcone/lightcone.h"
#include "unittest.h"

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
bool beacon_discovery4() {
    bool result = false;
    BeaconServer alice, bob;
    lightcone::SockAddr mcast_addr("239.0.0.4", 8888);
    if (!alice.init(mcast_addr, 1234, 1, 6000, 100, 5000)) return false;
    if (!bob.init(mcast_addr, 1234, 1, 6001, 100, 5000)) return false;
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
bool beacon_discovery6() {
    bool result = false;
    BeaconServer alice, bob;
    lightcone::SockAddr mcast_addr("ff05:0:0:0:0:0:0:2", 8888);
    if (!alice.init(mcast_addr, 1234, 1, 7000, 100, 5000)) return false;
    if (!bob.init(mcast_addr, 1234, 1, 7001, 100, 5000)) return false;
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
int main(int argc, char* argv[]) {
    lightcone::Network::start();
    UnitTest t(__FILE__);
    t.run("discovery ipv4", beacon_discovery4);
    // NOTE: travis-ci don't have ipv6 multicast
    // t.run("discovery ipv6",beacon_discovery6);
    lightcone::Network::stop();
    return t.failed ? 1 : 0;
}
// -----------------------------------------------------------
