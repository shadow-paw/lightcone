#include <string.h>
#include "udp.h"

namespace lightcone {
// -----------------------------------------------------------
Udp::Udp(Udp&& o) {
    m_socket = std::move(o.m_socket);
}
Udp& Udp::operator=(Udp&& o) {
    m_socket = std::move(o.m_socket);
    return *this;
}
bool Udp::open(int domain, bool nonblocking) {
    if (m_socket.is_valid()) return false;
    if (!m_socket.init(domain, SOCK_DGRAM, IPPROTO_UDP)) return false;
    if (!m_socket.set_nonblocking(nonblocking)) {
        m_socket.close();
        return false;
    } return true;
}
void Udp::close() {
    m_socket.close();
}
bool Udp::bind(const SockAddr& addr, bool reuse) {
    if (!m_socket.is_valid()) return false;
    if (reuse && !m_socket.set_reuse()) {
        m_socket.close();
        return false;
    }
    return m_socket.bind(addr);
}
bool Udp::bind(const std::string& addr, int port, bool reuse) {
    SockAddr a;
    if (!a.resolve(addr, port)) return false;
    return bind(a, reuse);
}
ssize_t Udp::send(const SockAddr& addr, const void* buf, size_t len) {
    auto p = addr.get_addr();
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    return (ssize_t)sendto(m_socket, (const char*)buf, static_cast<int>(len), 0, p.first, p.second);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
    return sendto(m_socket, buf, len, 0, p.first, p.second);
#else
    #error Not Implemented!
#endif
}
ssize_t Udp::recv(SockAddr* sender, void* buf, size_t len, unsigned int timeout) {
    if (!m_socket.is_valid()) return -1;
    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    FD_ZERO(&rfds);
    FD_SET(m_socket, &rfds);
    int events = select(static_cast<int>(m_socket+1), &rfds, NULL, NULL, &tv);
    if (events <= 0) return 0;
    socklen_t slen = sizeof(sender->m_addr);
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    return (ssize_t)recvfrom(m_socket, reinterpret_cast<char*>(buf), reinterpret_cast<int>(len), 0, reinterpret_cast<struct sockaddr*>(&sender->m_addr), &slen);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
    return recvfrom(m_socket, buf, len, 0, reinterpret_cast<struct sockaddr*>(&sender->m_addr), &slen);
#else
    #error Not Implemented!
#endif
}
bool Udp::joinmcast(const SockAddr& addr) {
    switch (addr.m_addr.base.sa_family) {
    case AF_INET: {
            struct ip_mreq mreq;
            memcpy (&mreq.imr_multiaddr.s_addr, &addr.m_addr.ip4.sin_addr, sizeof(struct in_addr));
            mreq.imr_interface.s_addr = 0;  // any
            return setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char*>(&mreq), sizeof(mreq)) == 0;
        }
    case AF_INET6: {
            struct ipv6_mreq mreq;
            memcpy(&mreq.ipv6mr_multiaddr, &addr.m_addr.ip6.sin6_addr, sizeof(struct in6_addr));
            mreq.ipv6mr_interface = 0;  // any
            return setsockopt(m_socket, IPPROTO_IPV6, IPV6_JOIN_GROUP, reinterpret_cast<char*>(&mreq), sizeof(mreq)) == 0;
        }
    }
    return false;
}
// -----------------------------------------------------------
}  // namespace lightcone
