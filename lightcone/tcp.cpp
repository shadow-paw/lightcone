#include <string.h>
#include <utility>
#include "tcp.h"

namespace lightcone {
// -----------------------------------------------------------
Tcp::Tcp() {
    protocol = 0;
    m_managed = false;
    m_state = State::Uninitalized;
    m_evmask = 0;
}
Tcp::~Tcp() {
}
Tcp::Tcp(Tcp&& o) {
    protocol = o.protocol; o.protocol = 0;
    ud = std::move(o.ud);
    m_socket = std::move(o.m_socket);
    m_ibuf = std::move(o.m_ibuf);
    m_obuf = std::move(o.m_obuf);
    m_state = o.m_state; o.m_state = State::Uninitalized;
}
Tcp& Tcp::operator=(Tcp&& o) {
    protocol = o.protocol; o.protocol = 0;
    ud = std::move(o.ud);
    m_socket = std::move(o.m_socket);
    m_ibuf = std::move(o.m_ibuf);
    m_obuf = std::move(o.m_obuf);
    m_state = o.m_state; o.m_state = State::Uninitalized;
    return *this;
}
bool Tcp::set_nonblocking(bool b) {
    return m_socket.set_nonblocking(b);
}
SockAddr Tcp::get_local() const {
    return m_socket.get_local();
}
SockAddr Tcp::get_remote() const {
    return m_socket.get_remote();
}
bool Tcp::listen(const SockAddr& addr, bool reuse) {
    if (m_socket.is_valid()) return false;
    if (!m_socket.init(addr.get_domain(), SOCK_STREAM, IPPROTO_TCP)) return false;
    if (!m_socket.set_nonblocking(true)) goto fail;
    if (reuse) {
        if (!m_socket.set_reuse()) goto fail;
    }
    if (!m_socket.bind(addr)) goto fail;
    if (!m_socket.listen()) goto fail;
    m_ibuf.free();
    m_obuf.free();
    m_state = State::Listening;
    return true;
fail:
    close();
    return false;
}
Tcp* Tcp::accept(SockAddr* addr, std::function<bool(const SockAddr& addr)> firewall) {
    Tcp* ret = nullptr;
    m_socket.accept(addr, [&ret, firewall](Socket&& accepted, const SockAddr& a) -> bool {
        if (firewall && !firewall(a)) return false;
        ret = new Tcp();
        ret->m_socket = std::move(accepted);
        ret->m_state = State::Accepted;
        return true;
    });
    return ret;
}
bool Tcp::connect(const SockAddr& addr, bool nonblocking, std::function<void(bool success)> cb) {
    if (m_socket.is_valid()) return false;
    m_ibuf.free();
    m_obuf.free();
    if (!m_socket.init(addr.get_domain(), SOCK_STREAM, IPPROTO_TCP)) return false;
    if (!m_socket.set_nonblocking(nonblocking)) goto fail;
    if (!m_socket.connect(addr)) {
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
        if (WSAEWOULDBLOCK != WSAGetLastError()) goto fail;
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
        if (errno != EINPROGRESS) goto fail;
#else
    #error Not Implemented!
#endif
        m_state = State::Connnecting;
        m_cb_connect = cb;
    } else {
        m_state = State::Connected;
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
    if (m_managed) {
        if (m_state == State::Connnecting) {
            m_state = State::Refused;
        } else if (m_state != State::Closed) {
            m_state = State::Closing;
        }
    } else {
        if (m_state == State::Connnecting) {
            if (m_cb_connect) {
                m_cb_connect(false);
                m_cb_connect = nullptr;
            }
        }
        m_socket.close();
        m_state = State::Closed;
    }
}
bool Tcp::send(const std::string& data) {
    uint8_t* wbuf = nullptr;
    size_t datalen = data.length();
    if (!m_obuf.reserve_begin(&wbuf, datalen)) return false;
    memcpy(wbuf, data.c_str(), datalen);
    m_obuf.reserve_end(datalen);
    return true;
}
bool Tcp::send(const void* data, size_t datalen) {
    uint8_t* wbuf = nullptr;
    if (!m_obuf.reserve_begin(&wbuf, datalen)) return false;
    memcpy(wbuf, data, datalen);
    m_obuf.reserve_end(datalen);
    return true;
}
bool Tcp::send(size_t reserve_size, std::function<ssize_t(uint8_t* wbuf, size_t wlen)> cb) {
    uint8_t* wbuf = nullptr;
    if (!m_obuf.reserve_begin(&wbuf, reserve_size)) return false;
    auto s = cb(wbuf, reserve_size);
    m_obuf.reserve_end(s < 0 ? 0 : (size_t)s);
    if (s < 0) close();
    return true;
}
bool Tcp::recv(std::function<size_t(const uint8_t* rbuf, size_t rlen)> cb) {
    size_t size = m_ibuf.size();
    if (size > 0) {
        size_t processed = cb(m_ibuf, size);
        m_ibuf.trim_head(processed);
    } return true;
}
int Tcp::poll(unsigned int milliseconds) {
    if (m_managed) return 0;
    if (!m_socket.is_valid()) return -1;
    fd_set rfds, wfds;
    struct timeval tv;
    FD_ZERO(&rfds); FD_ZERO(&wfds);
    FD_SET(m_socket, &rfds);
    if (m_state == State::Connnecting || m_obuf.size() > 0) FD_SET(m_socket, &wfds);
    tv.tv_sec = milliseconds / 1000;
    tv.tv_usec = (milliseconds % 1000) * 1000;
    int events = select(static_cast<int>(m_socket+1), &rfds, &wfds, NULL, &tv);
    if (events <= 0) return m_ibuf.size() > 0 ? 1 : 0;
    if (FD_ISSET(m_socket, &rfds)) io_read();
    if (m_socket.is_valid()) {
        if (FD_ISSET(m_socket, &wfds)) io_write();
    }
    return 1;
}
bool Tcp::io_read() {
    const size_t kRecvChunkSize = 8192;
    uint8_t* rbuf;
    size_t rlen = 0;
    ssize_t len;
    bool disconnected = false;
    if (!m_socket.is_valid()) return false;
    // Update state if connecting
    if (m_state == State::Connnecting) {
        m_state = State::Connected;
        if (m_cb_connect) {
            m_cb_connect(true);
            m_cb_connect = nullptr;
        }
    }
    while (!disconnected) {
        if (!m_ibuf.reserve_begin(&rbuf, kRecvChunkSize)) break;
        len = ::recv(m_socket, reinterpret_cast<char*>(rbuf), kRecvChunkSize, 0);
        if (len < 0) {
            m_ibuf.reserve_end(0);
            disconnected = m_socket.is_error();
            break;
        } else if (len == 0) {
            m_ibuf.reserve_end(0);
            disconnected = true;
            break;
        } else {
            rlen += (size_t)len;
            m_ibuf.reserve_end((size_t)len);
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
    if (!m_socket.is_valid()) return false;
    // Update state if connecting
    if (m_state == State::Connnecting) {
        m_state = State::Connected;
        if (m_cb_connect) {
            m_cb_connect(true);
            m_cb_connect = nullptr;
        }
    }
    wbuf = m_obuf;
    wlen = m_obuf.size();
    if (wlen > 0 && wbuf != nullptr) {
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
        len = (ssize_t)::send(m_socket, reinterpret_cast<char*>(buf), reinterpret_cast<int>(wlen), 0);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        len = ::send(m_socket, wbuf, wlen, MSG_NOSIGNAL);
#elif defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS) || defined(PLATFORM_SOLARIS)
        len = ::send(m_socket, wbuf, wlen, 0);
#else
    #error Not Implemented!
#endif
        if (len > 0) m_obuf.trim_head((size_t)len);
    }
    return true;
}
// -----------------------------------------------------------
}  // namespace lightcone
