#ifndef LIGHTCONE_NETWORK_H__
#define LIGHTCONE_NETWORK_H__

#include <string>
#include "netinc.h"

namespace lightcone {
// -----------------------------------------------------------
//! Start/Stop network facility
class Network {
 public:
    //! Initialize network, call this before other network functions
    static void start();
    //! Cleanup network
    static void stop();
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_NETWORK_H__
