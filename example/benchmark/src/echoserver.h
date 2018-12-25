#ifndef EXAMPLE_ECHOSERVER_H__
#define EXAMPLE_ECHOSERVER_H__

#include "lightcone/lightcone.h"

namespace example {
// -----------------------------------------------------------
class EchoServer : public lightcone::NetEngine {
 public:
    EchoServer();

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
};
// -----------------------------------------------------------
}  // namespace example

#endif  // EXAMPLE_ECHOSERVER_H__
