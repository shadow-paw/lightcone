#ifndef EXAMPLE_TESTER_H__
#define EXAMPLE_TESTER_H__

#include <stddef.h>
#include <atomic>
#include "lightcone/lightcone.h"

namespace example {
// -----------------------------------------------------------
class Tester : public lightcone::NetEngine {
 public:
     std::atomic<int> connects, packets, timeouts;

 public:
    Tester();
    ~Tester();
    bool setup(int concurrent, size_t packet_size);
    bool start(int threads);
    void stop();

 protected:
    virtual bool cb_net_timeout(lightcone::Tcp* conn, uint64_t now);
    virtual bool cb_net_refused(lightcone::Tcp* conn, uint64_t now);
    virtual bool cb_net_accepted(lightcone::Tcp* conn, lightcone::Tcp* from, uint64_t now);
    virtual bool cb_net_opened(lightcone::Tcp* conn, uint64_t now);
    virtual bool cb_net_closed(lightcone::Tcp* conn, uint64_t now);
    virtual bool cb_net_recv(lightcone::Tcp* conn, uint64_t now);
    virtual bool cb_net_sent(lightcone::Tcp* conn, uint64_t now);

 private:
    lightcone::LoadBalancerRR<uint32_t> _lb;
    bool _testing;
    int _concurrent;
    size_t _packet_size;
    unsigned char* _sendbuf;
};
// -----------------------------------------------------------
}  // namespace example

#endif  // EXAMPLE_TESTER_H__
