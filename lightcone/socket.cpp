#include <utility>
#include "socket.h"

namespace lightcone {
// -----------------------------------------------------------
Socket::Socket() {
    _fd = INVALID_SOCKET;
}
Socket::~Socket() {
    close();
}
Socket::Socket(const RAW_SOCKET& o) {
    _fd = o;
}
Socket::Socket(Socket&& o) {
    _fd = o._fd; o._fd = INVALID_SOCKET;
}
Socket& Socket::operator=(Socket&& o) {
    close();
    _fd = o._fd; o._fd = INVALID_SOCKET;
    return *this;
}
bool Socket::init(int domain, int type, int proto) {
    if (_fd != INVALID_SOCKET) return false;
    _fd = ::socket(domain, type, proto);
    if (_fd == INVALID_SOCKET) return false;
#if defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    int v = 1;
    setsockopt(_fd, SOL_SOCKET, SO_NOSIGPIPE, reinterpret_cast<char*>(&v), sizeof(v));
#endif
    return true;
}
bool Socket::bind(const SockAddr& addr) {
    auto p = addr.get_addr();
    return ::bind(_fd, p.first, p.second) == 0;
}
bool Socket::listen() {
    return ::listen(_fd, 128) == 0;
}
bool Socket::accept(SockAddr* addr, std::function<bool(Socket&& accepted, const SockAddr& addr)> handler) {
    struct sockaddr a;
    socklen_t slen = sizeof(a);
    RAW_SOCKET fd = ::accept(_fd, &a, &slen);
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
    return ::connect(_fd, p.first, p.second) == 0;
}
void Socket::close() {
    if (_fd != INVALID_SOCKET) {
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
        ::closesocket(_fd);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
        ::close(_fd);
#else
    #error Not Implemented!
#endif
        _fd = INVALID_SOCKET;
    }
}
SockAddr Socket::get_local() const {
    if (_fd == INVALID_SOCKET) return {};
    struct sockaddr addr;
    socklen_t slen = sizeof(addr);
    if (getsockname(_fd, &addr, &slen) != 0) return {};
    return SockAddr(addr);
}
SockAddr Socket::get_remote() const {
    if (_fd == INVALID_SOCKET) return {};
    struct sockaddr addr;
    socklen_t slen = sizeof(addr);
    if (getpeername(_fd, &addr, &slen) != 0) return {};
    return SockAddr(addr);
}
bool Socket::set_nonblocking(bool b) {
    if (_fd == INVALID_SOCKET) return false;
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    u_long a = b ? 1 : 0;
    ioctlsocket(_fd, FIONBIO, &a);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
    if (b) {
        fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL)|O_NONBLOCK);
    } else {
        fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL)& (~O_NONBLOCK));
    }
#else
    #error Not Implemented!
#endif
    return true;
}
bool Socket::set_reuse() {
    if (_fd == INVALID_SOCKET) return false;
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    int opt = 1;
    socklen_t slen = sizeof(opt);
    return setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&opt), slen) == 0;
#elif defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    int opt = 1;
    socklen_t slen = sizeof(opt);
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, slen) != 0) return false;
    return setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &opt, slen) == 0;
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
    int opt = 1;
    socklen_t slen = sizeof(opt);
    return setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, slen) == 0;
#else
    #error Not Implemented!
#endif
}
bool Socket::is_error() const {
    if (_fd == INVALID_SOCKET) return false;
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
    if (getsockopt(_fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), &slen) != 0) {
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
