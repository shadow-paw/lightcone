#include "socket.h"

namespace lightcone {
// -----------------------------------------------------------
Socket::Socket() {
    m_fd = INVALID_SOCKET;
}
Socket::~Socket() {
    close();
}
Socket::Socket(const RAW_SOCKET& o) {
    m_fd = o;
}
Socket::Socket(Socket&& o) {
    m_fd = o.m_fd; o.m_fd = INVALID_SOCKET;
}
Socket& Socket::operator=(Socket&& o) {
    close();
    m_fd = o.m_fd; o.m_fd = INVALID_SOCKET;
    return *this;
}
bool Socket::init(int domain, int type, int proto) {
    if (m_fd != INVALID_SOCKET) return false;
    m_fd = ::socket(domain, type, proto);
    if (m_fd == INVALID_SOCKET) return false;
#if defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    int v = 1;
    setsockopt(m_fd, SOL_SOCKET, SO_NOSIGPIPE, reinterpret_cast<char*>(&v), sizeof(v));
#endif
    return true;
}
bool Socket::bind(const SockAddr& addr) {
    auto p = addr.get_addr();
    return ::bind(m_fd, p.first, p.second) == 0;
}
bool Socket::listen() {
    return ::listen(m_fd, 128) == 0;
}
bool Socket::accept(SockAddr* addr, std::function<bool(Socket&& accepted, const SockAddr& addr)> handler) {
    struct sockaddr a;
    socklen_t slen = sizeof(a);
    RAW_SOCKET fd = ::accept(m_fd, &a, &slen);
    if (fd == INVALID_SOCKET) return false;
    Socket accepted(fd);
    if (addr) {
        *addr = SockAddr(a);
        if (handler) handler(std::move(accepted), *addr);
    } else {
        if (handler) handler(std::move(accepted), SockAddr(a));
    }
    return true;
}
bool Socket::connect(const SockAddr& addr) {
    auto p = addr.get_addr();
    return ::connect(m_fd, p.first, p.second) == 0;
}
void Socket::close() {
    if (m_fd != INVALID_SOCKET) {
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
        ::closesocket(m_fd);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
        ::close(m_fd);
#else
    #error Not Implemented!
#endif
        m_fd = INVALID_SOCKET;
    }
}
SockAddr Socket::get_local() const {
    SockAddr addr;
    if (m_fd == INVALID_SOCKET) return addr;
    socklen_t slen = sizeof(addr.m_addr);
    if (getsockname(m_fd, &addr.m_addr.base, &slen) != 0) return addr;
    return addr;
}
SockAddr Socket::get_remote() const {
    SockAddr addr;
    if (m_fd == INVALID_SOCKET) return addr;
    socklen_t slen = sizeof(addr.m_addr);
    if (getpeername(m_fd, &addr.m_addr.base, &slen) != 0) return addr;
    return addr;
}
bool Socket::set_nonblocking(bool b) {
    if (m_fd == INVALID_SOCKET) return false;
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    uint64_t a = b ? 1 : 0;
    ioctlsocket(m_fd, FIONBIO, &a);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
    if (b) {
        fcntl(m_fd, F_SETFL, fcntl(m_fd, F_GETFL)|O_NONBLOCK);
    } else {
        fcntl(m_fd, F_SETFL, fcntl(m_fd, F_GETFL)& (~O_NONBLOCK));
    }
#else
    #error Not Implemented!
#endif
    return true;
}
bool Socket::set_reuse() {
    if (m_fd == INVALID_SOCKET) return false;
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    int opt = 1;
    socklen_t slen = sizeof(opt);
    return setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&opt), slen) == 0;
#elif defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    int opt = 1;
    socklen_t slen = sizeof(opt);
    if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &opt, slen) != 0) return false;
    return setsockopt(m_fd, SOL_SOCKET, SO_REUSEPORT, &opt, slen) == 0;
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
    int opt = 1;
    socklen_t slen = sizeof(opt);
    return setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &opt, slen) == 0;
#else
    #error Not Implemented!
#endif
}
bool Socket::is_error() const {
    if (m_fd == INVALID_SOCKET) return false;
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    switch (WSAGetLastError()) {
    case WSAENETDOWN:
    case WSAENETRESET:
    case WSAEHOSTUNREACH:
    case WSAECONNABORTED:
    case WSAECONNRESET:
    case WSAETIMEDOUT:
        return true;
//    case WSAEINPROGRESS:
//    case WSAENOBUFS:
//    case WSAEWOULDBLOCK:
//        return false;
  } return false;
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
    int err;
    socklen_t slen = sizeof(err);
    if (getsockopt(m_fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), &slen) != 0) {
        err = EHOSTDOWN;
    }
    switch (err) {
    case ENOTCONN:
    case EHOSTDOWN:
    case EPIPE:
        return true;
//    case EAGAIN:          // Resource temporarily unavailable
//    case EWOULDBLOCK:     // Operation would block
//    case EINPROGRESS:     // Operation now in progress
//    case EINTR:               // Interrupt occured, can recover
//    case ENOBUFS:         // System unable to allocate buf now, can recover
//        return false;
    } return false;
#else
    #error Not Implemented!
#endif
}
// -----------------------------------------------------------
}  // namespace lightcone
