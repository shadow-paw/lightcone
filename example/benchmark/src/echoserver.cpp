#include "echoserver.h"

namespace example {
// -----------------------------------------------------------
EchoServer::EchoServer() : lightcone::NetEngine(&m_lb) {
    // set_timeout(1000);
}
// -----------------------------------------------------------
bool EchoServer::cb_net_timeout(lightcone::Tcp* conn, uint64_t now) {
    return true;
}
// -----------------------------------------------------------
bool EchoServer::cb_net_refused(lightcone::Tcp* conn, uint64_t now) {
    return true;
}
// -----------------------------------------------------------
bool EchoServer::cb_net_accepted(lightcone::Tcp* conn, uint64_t now) {
    return true;
}
// -----------------------------------------------------------
bool EchoServer::cb_net_opened(lightcone::Tcp* conn, uint64_t now) {
    return true;
}
// -----------------------------------------------------------
bool EchoServer::cb_net_closed(lightcone::Tcp* conn, uint64_t now) {
    return true;
}
// -----------------------------------------------------------
bool EchoServer::cb_net_recv(lightcone::Tcp* conn, uint64_t now) {
    conn->recv([this, conn](const uint8_t* rbuf, size_t rlen) -> size_t {
        conn->send(rbuf, rlen);
        return rlen;
    });
    return true;
}
// -----------------------------------------------------------
bool EchoServer::cb_net_sent(lightcone::Tcp* conn, uint64_t now) {
    return true;
}
// -----------------------------------------------------------
}  // namespace example
