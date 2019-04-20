#include <string.h>
#include <utility>
#include "tcp.h"

namespace lightcone {
// -----------------------------------------------------------
Tcp::Tcp() {
    protocol = 0;
    _managed = false;
    _state = State::Uninitalized;
    _evmask = 0;
}
Tcp::~Tcp() {
}
Tcp::Tcp(Tcp&& o) {
    protocol = o.protocol; o.protocol = 0;
    ud = std::move(o.ud);
    _socket = std::move(o._socket);
    _ibuf = std::move(o._ibuf);
    _obuf = std::move(o._obuf);
    _state = o._state; o._state = State::Uninitalized;
}
Tcp& Tcp::operator=(Tcp&& o) {
    protocol = o.protocol; o.protocol = 0;
    ud = std::move(o.ud);
    _socket = std::move(o._socket);
    _ibuf = std::move(o._ibuf);
    _obuf = std::move(o._obuf);
    _state = o._state; o._state = State::Uninitalized;
    return *this;
}
bool Tcp::set_nonblocking(bool b) {
    return _socket.set_nonblocking(b);
}
bool Tcp::set_keepalive(bool enable, int idle, int interval, int probes) {
    if (enable) {
        int opt = 1;
        if (setsockopt(_socket, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&opt), (socklen_t)sizeof(opt)) != 0) return false;
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64) || defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID) || defined(PLATFORM_BSD)
        // NOTE: Not supported on Mac OS X
        if (setsockopt(_socket, IPPROTO_TCP, TCP_KEEPIDLE, reinterpret_cast<const char*>(&idle), (socklen_t)sizeof(idle)) != 0) return false;
        if (setsockopt(_socket, IPPROTO_TCP, TCP_KEEPINTVL, reinterpret_cast<const char*>(&interval), (socklen_t)sizeof(interval)) != 0) return false;
        if (setsockopt(_socket, IPPROTO_TCP, TCP_KEEPCNT, reinterpret_cast<const char*>(&probes), (socklen_t)sizeof(probes)) != 0) return false;
#endif
    } else {
        int opt = 0;
        if (setsockopt(_socket, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&opt), (socklen_t)sizeof(opt)) != 0) return false;
    }
    return true;
}
SockAddr Tcp::get_local() const {
    return _socket.get_local();
}
SockAddr Tcp::get_remote() const {
    return _socket.get_remote();
}
bool Tcp::listen(const SockAddr& addr, bool reuse) {
    if (_socket.is_valid()) return false;
    if (!_socket.init(addr.get_family(), SOCK_STREAM, IPPROTO_TCP)) return false;
    if (!_socket.set_nonblocking(true)) goto fail;
    if (reuse) {
        if (!_socket.set_reuse()) goto fail;
    }
    if (!_socket.bind(addr)) goto fail;
    if (!_socket.listen()) goto fail;
    _ibuf.free();
    _obuf.free();
    _state = State::Listening;
    return true;
fail:
    close();
    return false;
}
Tcp* Tcp::accept(SockAddr* addr, std::function<bool(const SockAddr& addr)> firewall) {
    Tcp* ret = nullptr;
    _socket.accept(addr, [&ret, firewall](Socket&& accepted, const SockAddr& a) -> bool {
        if (firewall && !firewall(a)) return false;
        ret = new Tcp();
        ret->_socket = std::move(accepted);
        ret->_state = State::Accepted;
        return true;
    });
    return ret;
}
bool Tcp::connect(const SockAddr& addr, bool nonblocking, std::function<void(bool success)> cb) {
    if (_socket.is_valid()) return false;
    _ibuf.free();
    _obuf.free();
    if (!_socket.init(addr.get_family(), SOCK_STREAM, IPPROTO_TCP)) return false;
    if (!_socket.set_nonblocking(nonblocking)) goto fail;
    if (!_socket.connect(addr)) {
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
        if (WSAEWOULDBLOCK != WSAGetLastError()) goto fail;
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
        if (errno != EINPROGRESS) goto fail;
#else
    #error Not Implemented!
#endif
        _state = State::Connnecting;
        _cb_connect = cb;
    } else {
        _state = State::Connected;
        if (cb) cb(true);
    }
    return true;
fail:
    close();
    return false;
}
bool Tcp::connect(const std::string& hostname, int port, bool nonblocking, std::function<void(bool success)> cb) {
    SockAddr addr;
    if (!addr.resolve(hostname, port)) return false;
    return connect(addr, nonblocking, cb);
}
void Tcp::close() {
    if (_managed) {
        if (_state == State::Connnecting) {
            _state = State::Refused;
        } else if (_state != State::Closed) {
            _state = State::Closing;
        }
    } else {
        if (_state == State::Connnecting) {
            if (_cb_connect) {
                _cb_connect(false);
                _cb_connect = nullptr;
            }
        }
        _socket.close();
        _state = State::Closed;
    }
}
bool Tcp::send(const std::string& data) {
    uint8_t* wbuf = nullptr;
    size_t datalen = data.length();
    if (!_obuf.reserve_begin(&wbuf, datalen)) return false;
    memcpy(wbuf, data.c_str(), datalen);
    _obuf.reserve_end(datalen);
    return true;
}
bool Tcp::send(const void* data, size_t datalen) {
    uint8_t* wbuf = nullptr;
    if (!_obuf.reserve_begin(&wbuf, datalen)) return false;
    memcpy(wbuf, data, datalen);
    _obuf.reserve_end(datalen);
    return true;
}
bool Tcp::send(size_t reserve_size, std::function<ssize_t(uint8_t* wbuf, size_t wlen)> cb) {
    uint8_t* wbuf = nullptr;
    if (!_obuf.reserve_begin(&wbuf, reserve_size)) return false;
    auto s = cb(wbuf, reserve_size);
    _obuf.reserve_end(s < 0 ? 0 : (size_t)s);
    if (s < 0) close();
    return true;
}
bool Tcp::recv(std::function<size_t(const uint8_t* rbuf, size_t rlen)> cb) {
    size_t size = _ibuf.size();
    if (size > 0) {
        size_t processed = cb(_ibuf, size);
        _ibuf.trim_head(processed);
    } return true;
}
int Tcp::poll(unsigned int milliseconds) {
    if (_managed) return 0;
    if (!_socket.is_valid()) return -1;
    fd_set rfds, wfds;
    struct timeval tv;
    FD_ZERO(&rfds); FD_ZERO(&wfds);
    FD_SET(_socket, &rfds);
    if (_state == State::Connnecting || _obuf.size() > 0) FD_SET(_socket, &wfds);
    tv.tv_sec = milliseconds / 1000;
    tv.tv_usec = (milliseconds % 1000) * 1000;
    int events = select(static_cast<int>(_socket+1), &rfds, &wfds, NULL, &tv);
    if (events <= 0) return _ibuf.size() > 0 ? 1 : 0;
    if (FD_ISSET(_socket, &rfds)) io_read();
    if (_socket.is_valid()) {
        if (FD_ISSET(_socket, &wfds)) io_write();
    }
    return 1;
}
bool Tcp::io_read() {
    const size_t kRecvChunkSize = 8192;
    uint8_t* rbuf;
    size_t rlen = 0;
    ssize_t len;
    bool disconnected = false;
    if (!_socket.is_valid()) return false;
    // Update state if connecting
    if (_state == State::Connnecting) {
        _state = State::Connected;
        if (_cb_connect) {
            _cb_connect(true);
            _cb_connect = nullptr;
        }
    }
    while (!disconnected) {
        if (!_ibuf.reserve_begin(&rbuf, kRecvChunkSize)) break;
        len = ::recv(_socket, reinterpret_cast<char*>(rbuf), kRecvChunkSize, 0);
        if (len < 0) {
            _ibuf.reserve_end(0);
            disconnected = _socket.is_error();
            break;
        } else if (len == 0) {
            _ibuf.reserve_end(0);
            disconnected = true;
            break;
        } else {
            rlen += (size_t)len;
            _ibuf.reserve_end((size_t)len);
            if ((size_t)len < kRecvChunkSize) break;
        }
    }
    if (disconnected) close();
    return rlen > 0;
}
bool Tcp::io_write() {
    uint8_t* wbuf;
    size_t wlen;
    ssize_t len;
    if (!_socket.is_valid()) return false;
    // Update state if connecting
    if (_state == State::Connnecting) {
        _state = State::Connected;
        if (_cb_connect) {
            _cb_connect(true);
            _cb_connect = nullptr;
        }
    }
    wbuf = _obuf;
    wlen = _obuf.size();
    if (wlen > 0 && wbuf != nullptr) {
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
        len = (ssize_t)::send(_socket, reinterpret_cast<char*>(wbuf), static_cast<int>(wlen), 0);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        len = ::send(_socket, wbuf, wlen, MSG_NOSIGNAL);
#elif defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS) || defined(PLATFORM_SOLARIS)
        len = ::send(_socket, wbuf, wlen, 0);
#else
    #error Not Implemented!
#endif
        if (len > 0) _obuf.trim_head((size_t)len);
    }
    return true;
}
// -----------------------------------------------------------
}  // namespace lightcone
