#ifndef LIGHTCONE_HTTPSERVER_H__
#define LIGHTCONE_HTTPSERVER_H__

#include <stdint.h>
#include <stddef.h>
#include "copyable.h"
#include "tcp.h"
#include "httpheader.h"

namespace lightcone {
// -----------------------------------------------------------
//! HTTP Server Helper, use with NetEngine
class HttpServer : private NonCopyable<HttpServer> {
 protected:
    /// delegate event
    virtual bool cb_http(Tcp* conn, const HttpRequestHeader& header, const uint8_t* data, size_t datalen, uint64_t now) = 0;

 protected:
    bool http_response(Tcp* conn, int status, const char* text = nullptr);
    bool http_response(Tcp* conn, const HttpResponseHeader& header, const void* data, size_t datalen);

 protected:
    bool http_timeout(Tcp* conn, uint64_t now);
    bool http_accepted(Tcp* conn, uint64_t now);
    bool http_closed(Tcp* conn, uint64_t now);
    bool http_recv(Tcp* conn, uint64_t now);
    bool http_sent(Tcp* conn, uint64_t now);

 private:
    struct ConnectionData {
        ssize_t header_len;
        ssize_t content_len;
        HttpRequestHeader header;
        ConnectionData(): header_len(0), content_len(-1) {}
    };
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_HTTPSERVER_H__
