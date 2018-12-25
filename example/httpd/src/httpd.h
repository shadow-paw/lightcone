#ifndef EXAMPLE_HTTPD_H__
#define EXAMPLE_HTTPD_H__

#include "lightcone/lightcone.h"

namespace example {
// -----------------------------------------------------------
class HttpServer : public lightcone::NetEngine, protected lightcone::HttpServer {
 public:
    HttpServer();

 protected:
    virtual bool cb_http(lightcone::Tcp* conn,
                         const lightcone::HttpRequestHeader& header,
                         const uint8_t* data, size_t datalen,
                         uint64_t now);

 protected:
    bool cb_net_timeout(lightcone::Tcp* conn, uint64_t now);
    bool cb_net_refused(lightcone::Tcp* conn, uint64_t now);
    bool cb_net_accepted(lightcone::Tcp* conn, lightcone::Tcp* from, uint64_t now);
    bool cb_net_opened(lightcone::Tcp* conn, uint64_t now);
    bool cb_net_closed(lightcone::Tcp* conn, uint64_t now);
    bool cb_net_recv(lightcone::Tcp* conn, uint64_t now);
    bool cb_net_sent(lightcone::Tcp* conn, uint64_t now);

 private:
    lightcone::LoadBalancerRR<uint32_t> _lb;
};
// -----------------------------------------------------------
}  // namespace example

#endif  // EXAMPLE_HTTPD_H__
