#include "httpd.h"

namespace example {
// -----------------------------------------------------------
HttpServer::HttpServer() : lightcone::NetEngine(&_lb), lightcone::HttpServer() {
    set_timeout(5000);
}
bool HttpServer::cb_net_timeout(lightcone::Tcp* conn, uint64_t now) {
    return true;
}
bool HttpServer::cb_net_refused(lightcone::Tcp* conn, uint64_t now) {
    return true;
}
bool HttpServer::cb_net_accepted(lightcone::Tcp* conn, lightcone::Tcp* from, uint64_t now) {
    // NOTE: To support multiple protocol, insert logic here
    conn->protocol = 'http';
    switch (conn->protocol) {
    case 'http':
        return http_accepted(conn, now);
    default:
        return false;
    }
}
bool HttpServer::cb_net_opened(lightcone::Tcp* conn, uint64_t now) {
    return true;
}
bool HttpServer::cb_net_closed(lightcone::Tcp* conn, uint64_t now) {
    switch (conn->protocol) {
    case 'http':
        return http_closed(conn, now);
    default:
        return false;
    }
}
bool HttpServer::cb_net_recv(lightcone::Tcp* conn, uint64_t now) {
    switch (conn->protocol) {
    case 'http':
        return http_recv(conn, now);
    default:
        return false;
    }
}
bool HttpServer::cb_net_sent(lightcone::Tcp* conn, uint64_t now) {
    switch (conn->protocol) {
    case 'http':
        return http_sent(conn, now);
    default:
        return false;
    }
}
bool HttpServer::cb_http(lightcone::Tcp* conn, const lightcone::HttpRequestHeader& header, const uint8_t* data, size_t datalen, uint64_t now) {
    if (header.uri.compare("/foo") == 0) {
        return http_response(conn, 200, "bar");
    }
    return false;
}
// -----------------------------------------------------------
}  // namespace example
