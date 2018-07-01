#include <string.h>
#include <stdexcept>
#include "sockaddr.h"

namespace lightcone {
// -----------------------------------------------------------
SockAddr::SockAddr(const std::string& ip, int port) {
    if (ip.find(".") != std::string::npos) {
        if (!ip4(ip, port)) throw std::invalid_argument("ip");
    } else {
        if (!ip6(ip, port)) throw std::invalid_argument("ip");
    }
}
// -----------------------------------------------------------
SockAddr::SockAddr(const struct sockaddr& o) {
    m_addr = o;
}
// -----------------------------------------------------------
SockAddr::SockAddr(const SockAddr& o) {
    m_addr = o.m_addr;
}
// -----------------------------------------------------------
SockAddr::SockAddr(SockAddr&& o) {
    m_addr = o.m_addr;
}
// -----------------------------------------------------------
SockAddr& SockAddr::operator=(const SockAddr& o) {
    m_addr = o.m_addr;
    return *this;
}
// -----------------------------------------------------------
SockAddr& SockAddr::operator=(SockAddr&& o) {
    m_addr = o.m_addr;
    return *this;
}
// -----------------------------------------------------------
uint32_t SockAddr::get_ip4() const {
    switch (m_addr.sa_family) {
    case AF_INET: {
            uint32_t ip4;
            memcpy(&ip4, &m_addr.sa_data[2], sizeof(ip4));
            return ip4;
        }
    } return 0;
}
// -----------------------------------------------------------
int SockAddr::get_port() const {
    switch (m_addr.sa_family) {
    case AF_INET: {
            const struct sockaddr& a = m_addr;
            const struct sockaddr_in* sa = reinterpret_cast<const struct sockaddr_in*>(&a);
            return ntohs(sa->sin_port);
        }
    case AF_INET6: {
            const struct sockaddr& a = m_addr;
            const struct sockaddr_in6* sa = reinterpret_cast<const struct sockaddr_in6*>(&a);
            return ntohs(sa->sin6_port);
        }
    default:
        return 0;
    }
}
// -----------------------------------------------------------
void SockAddr::set_port(int port) {
    switch (m_addr.sa_family) {
    case AF_INET: {
            struct sockaddr& a = m_addr;
            struct sockaddr_in* sa = reinterpret_cast<struct sockaddr_in*>(&a);
            sa->sin_port = htons(port);
        }
    case AF_INET6: {
            struct sockaddr& a = m_addr;
            struct sockaddr_in6* sa = reinterpret_cast<struct sockaddr_in6*>(&a);
            sa->sin6_port = htons(port);
        }
    }
}
// -----------------------------------------------------------
std::string SockAddr::to_string() const {
    char buffer[INET_ADDRSTRLEN] = {};
    int port = 0;
    switch (m_addr.sa_family) {
    case AF_INET: {
            const struct sockaddr& a = m_addr;
            const struct sockaddr_in* sa = reinterpret_cast<const struct sockaddr_in*>(&a);
            inet_ntop(AF_INET, &sa->sin_addr, buffer, INET_ADDRSTRLEN);
            port = ntohs(sa->sin_port);
            break;
        }
    case AF_INET6: {
            const struct sockaddr& a = m_addr;
            const struct sockaddr_in6* sa = reinterpret_cast<const struct sockaddr_in6*>(&a);
            inet_ntop(AF_INET6, &sa->sin6_addr, buffer, INET_ADDRSTRLEN);
            port = ntohs(sa->sin6_port);
            break;
        }
    default:
        return "Invalid AF";
    } return std::string(buffer) + ":" + std::to_string(port);
}
// -----------------------------------------------------------
bool SockAddr::ip4(const std::string& ip, int port) {
    struct sockaddr_in* sa = reinterpret_cast<struct sockaddr_in*>(&m_addr);
    if (inet_pton(AF_INET, ip.c_str(), &sa->sin_addr) != 1) return false;
    sa->sin_family = AF_INET;
    sa->sin_port = htons((uint16_t)port);
    return true;
}
// -----------------------------------------------------------
bool SockAddr::ip6(const std::string& ip, int port) {
    struct sockaddr_in6* sa = reinterpret_cast<struct sockaddr_in6*>(&m_addr);
    if (inet_pton(AF_INET6, ip.c_str(), &sa->sin6_addr) != 1) return false;
    sa->sin6_family = AF_INET6;
    sa->sin6_port = htons((uint16_t)port);
    return true;
}
// -----------------------------------------------------------
bool SockAddr::resolve(const std::string& host, int default_port) {
    struct addrinfo hints = {};
    struct addrinfo* addrs = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    auto port_pos = host.find(":");
    if (port_pos != std::string::npos) {
        default_port = atoi(host.substr(port_pos + 1).c_str());
        if (getaddrinfo(host.substr(0, port_pos).c_str(), NULL, &hints, &addrs) != 0) return false;
    } else {
        if (getaddrinfo(host.c_str(), NULL, &hints, &addrs) != 0) return false;
    }
    if (!addrs) return false;
    memcpy(&m_addr, addrs->ai_addr, addrs->ai_addrlen);
    uint16_t temp = (uint16_t)htons((uint16_t)default_port);
    memcpy(m_addr.sa_data, &temp, sizeof(temp));
    freeaddrinfo(addrs);
    return true;
}
// -----------------------------------------------------------
bool SockAddr::is_multicast() const {
    switch (m_addr.sa_family) {
    case AF_INET:
        return ((unsigned char)m_addr.sa_data[2] & 0xf0) == 0xe0;  // 224~239
    case AF_INET6:
        return ((unsigned char)m_addr.sa_data[2] == 0xff);  // high octet is 0xff
    } return false;
}
// -----------------------------------------------------------
uint32_t SockAddr::hash() const {
    uint32_t ret = 0;
    switch (m_addr.sa_family) {
    case AF_INET:
        memcpy(&ret, &m_addr.sa_data[2], sizeof(ret));
        break;
    case AF_INET6:
        // TODO: improve this
        memcpy(&ret, &m_addr.sa_data[2], sizeof(ret));
        break;
    } return ret;
}
// -----------------------------------------------------------
}  // namespace lightcone
