#include <string.h>
#include "udp.h"

// -----------------------------------------------------------
lightcone::Udp::Udp(lightcone::Udp&& o) {
    m_socket = std::move(o.m_socket);
}
// -----------------------------------------------------------
lightcone::Udp& lightcone::Udp::operator=(lightcone::Udp&& o) {
    m_socket = std::move(o.m_socket);
    return *this;
}
// -----------------------------------------------------------
bool lightcone::Udp::open(bool nonblocking) {
    if (m_socket.is_valid()) return false;
    if (!m_socket.init(SOCK_DGRAM, IPPROTO_UDP)) return false;
    if (!m_socket.set_nonblocking(nonblocking)) {
        m_socket.close();
        return false;
    } return true;
}
// -----------------------------------------------------------
void lightcone::Udp::close() {
    m_socket.close();
}
// -----------------------------------------------------------
bool lightcone::Udp::bind(const lightcone::SockAddr& addr, bool reuse) {
    if (!m_socket.is_valid()) return false;
    if (reuse) {
        if (!m_socket.set_reuse()) return false;
    }
    return m_socket.bind(addr);
}
// -----------------------------------------------------------
bool lightcone::Udp::bind(const std::string& addr, int port, bool reuse) {
    SockAddr a;
    if (!a.resolve(addr, port)) return false;
    return bind(a, reuse);
}
// -----------------------------------------------------------
ssize_t lightcone::Udp::send(const lightcone::SockAddr& addr, const void* buf, size_t len) {
    struct sockaddr a = addr;
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    return (ssize_t)sendto(m_socket, (const char*)buf, reinterpret_cast<int>(len), 0, &a, sizeof(struct sockaddr));
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_OSX) || defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
    return sendto(m_socket, buf, len, 0, &a, sizeof(struct sockaddr));
#else
    #error Not Implemented!
#endif
}
// -----------------------------------------------------------
ssize_t lightcone::Udp::recv(lightcone::SockAddr* sender, void* buf, size_t len, unsigned int timeout) {
    if (!m_socket.is_valid()) return -1;
    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    FD_ZERO(&rfds);
    FD_SET(m_socket, &rfds);
    int events = select(static_cast<int>(m_socket+1), &rfds, NULL, NULL, &tv);
    if (events <= 0) return 0;
    socklen_t slen = sizeof(struct sockaddr);
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    return (ssize_t)recvfrom(m_socket, reinterpret_cast<char*>(buf), reinterpret_cast<int>(len), 0, &sender->operator struct sockaddr&(), &slen);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_OSX) || defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
    return recvfrom(m_socket, buf, len, 0, &sender->operator struct sockaddr&(), &slen);
#else
    #error Not Implemented!
#endif
}
// -----------------------------------------------------------
bool lightcone::Udp::joinmcast(const lightcone::SockAddr& addr) {
    struct sockaddr a = addr;
    switch (a.sa_family) {
    case AF_INET: {
            struct sockaddr_in* sa = reinterpret_cast<struct sockaddr_in*>(&a);
            struct ip_mreq mreq;
            memcpy (&mreq.imr_multiaddr.s_addr, &sa->sin_addr, sizeof(sa->sin_addr));
            mreq.imr_interface.s_addr = 0;  // any
            return setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char*>(&mreq), sizeof(mreq)) == 0;
        }
    case AF_INET6: {
            struct sockaddr_in6* sa = reinterpret_cast<struct sockaddr_in6*>(&a);
            struct ipv6_mreq mreq;
            memcpy(&mreq.ipv6mr_multiaddr, &sa->sin6_addr, sizeof(sa->sin6_addr));
            mreq.ipv6mr_interface = 0;  // any
            return setsockopt(m_socket, IPPROTO_IPV6, IPV6_JOIN_GROUP, reinterpret_cast<char*>(&mreq), sizeof(mreq)) == 0;
        }
    }
    return false;
}
// -----------------------------------------------------------
