#include <string.h>
#include <utility>
#include <iostream>
#include "httpserver.h"

namespace lightcone {
// -----------------------------------------------------------
bool HttpServer::http_timeout(Tcp* conn, uint64_t now) {
    conn->close();
    return true;
}
bool HttpServer::http_accepted(Tcp* conn, uint64_t now) {
    ConnectionData* data = new ConnectionData();
    if (!data) return false;
    conn->ud.emplace(std::make_pair('_htp', reinterpret_cast<uintptr_t>(data)));
    return true;
}
bool HttpServer::http_closed(Tcp* conn, uint64_t now) {
    if (auto it=conn->ud.find('_htp'); it != conn->ud.end()) {
        ConnectionData* data = reinterpret_cast<ConnectionData*>(it->second);
        delete data;
        conn->ud.erase(it);
    }
    return true;
}
bool HttpServer::http_recv(Tcp* conn, uint64_t now) {
    return conn->recv([this, &conn, now](const uint8_t* rbuf, size_t rlen) -> ssize_t {
        ConnectionData* data = nullptr;
        // connection data
        if (auto it=conn->ud.find('_htp'); it != conn->ud.end()) {
            data = reinterpret_cast<ConnectionData*>(it->second);
        }
        if (!data) return -1;

        if (data->header_len == 0) {
            data->header_len = data->header.read_from(rbuf, rlen);
            if (data->header_len == 0) return 0;
            if (auto content_len = data->header.find("content-length"); content_len != data->header.end()) {
                data->content_len = std::atoi(content_len->second.c_str());
            } else {
                data->content_len = 0;
            }
        }
        if (data->header_len < 0) return -1;
        if (data->content_len < 0) return -1;

        if (rlen >= (size_t)(data->header_len + data->content_len)) {
            if (!cb_http(conn, data->header, rbuf + data->header_len, rlen - (size_t)data->header_len, now)) {
                http_response(conn, 404);
            }
            return data->header_len + data->content_len;
        }
        // not fully received, wait for next event
        return 0;
    });
}
bool HttpServer::http_sent(Tcp* conn, uint64_t now) {
    return true;
}
bool HttpServer::http_response(Tcp* conn, int status, const char* text) {
    HttpResponseHeader res(status);
    size_t content_len = text ? strlen(text) : 0;
    res.emplace(std::pair("content-length", std::to_string(content_len)));
    return http_response(conn, res, text, content_len);
}
bool HttpServer::http_response(Tcp* conn, const HttpResponseHeader& header, const void* data, size_t datalen) {
    if (data && !datalen) return false;
    return conn->send(16384 + datalen, [&header, data, datalen](uint8_t* wbuf, size_t wlen) -> ssize_t {
        auto header_len = header.write_to(wbuf, 16384);
        if (header_len < 0) return -1;
        if (data) {
            if ((size_t)header_len + datalen > wlen) return -1;
            memcpy(wbuf + header_len, data, datalen);
            return header_len + (ssize_t)datalen;
        }
        return header_len;
    });
}
// -----------------------------------------------------------
}  // namespace lightcone
