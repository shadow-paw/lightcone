#include <string.h>
#include <stdexcept>
#include "sockaddr.h"

namespace lightcone {
// -----------------------------------------------------------
SockAddr::SockAddr(const std::string& ip, int port) {
    if (!set_ip(ip, port)) throw std::invalid_argument("ip");
}
// -----------------------------------------------------------
SockAddr::SockAddr(const struct sockaddr& o) {
    m_addr.base = o;
}
// -----------------------------------------------------------
SockAddr::SockAddr(const struct sockaddr_in& o) {
    m_addr.ip4 = o;
}
// -----------------------------------------------------------
SockAddr::SockAddr(const struct sockaddr_in6& o) {
    m_addr.ip6 = o;
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
std::pair<const struct sockaddr*, socklen_t> SockAddr::get_addr() const {
    switch (m_addr.base.sa_family) {
    case AF_INET:
        return std::make_pair(reinterpret_cast<const struct sockaddr*>(&m_addr.ip4), sizeof(m_addr.ip4));
    case AF_INET6:
        return std::make_pair(reinterpret_cast<const struct sockaddr*>(&m_addr.ip6), sizeof(m_addr.ip6));
    default:
        return std::make_pair(nullptr, 0);
    }
}
// -----------------------------------------------------------
uint32_t SockAddr::get_ip4() const {
    switch (m_addr.base.sa_family) {
    case AF_INET: {
            uint32_t ip4;
            memcpy(&ip4, &m_addr.ip4.sin_port, sizeof(ip4));
            return ip4;
        }
    } return 0;
}
// -----------------------------------------------------------
int SockAddr::get_port() const {
    switch (m_addr.base.sa_family) {
    case AF_INET:  return ntohs(m_addr.ip4.sin_port);
    case AF_INET6: return ntohs(m_addr.ip6.sin6_port);
    default:       return 0;
    }
}
// -----------------------------------------------------------
void SockAddr::set_port(int port) {
    switch (m_addr.base.sa_family) {
    case AF_INET:
        m_addr.ip4.sin_port = htons(port);
        break;
    case AF_INET6:
        m_addr.ip6.sin6_port = htons(port);
        break;
    }
}
// -----------------------------------------------------------
std::string SockAddr::to_string() const {
    char buffer[INET_ADDRSTRLEN] = {};
    int port = 0;
    switch (m_addr.base.sa_family) {
    case AF_INET:
        inet_ntop(AF_INET, &m_addr.ip4.sin_addr, buffer, INET_ADDRSTRLEN);
        port = ntohs(m_addr.ip4.sin_port);
        break;
    case AF_INET6:
        inet_ntop(AF_INET6, &m_addr.ip6.sin6_addr, buffer, INET_ADDRSTRLEN);
        port = ntohs(m_addr.ip6.sin6_port);
        break;
    default:
        return "Invalid AF";
    } return std::string(buffer) + ":" + std::to_string(port);
}
// -----------------------------------------------------------
bool SockAddr::set_ip(const std::string& ip, int port) {
    if (ip.find(".") != std::string::npos) {
        return set_ip4(ip, port);
    } else {
        return set_ip6(ip, port);
    }
}
// -----------------------------------------------------------
bool SockAddr::set_ip4(const std::string& ip, int port) {
    if (inet_pton(AF_INET, ip.c_str(), &m_addr.ip4.sin_addr) != 1) return false;
    m_addr.ip4.sin_family = AF_INET;
    m_addr.ip4.sin_port = htons((uint16_t)port);
    return true;
}
// -----------------------------------------------------------
bool SockAddr::set_ip6(const std::string& ip, int port) {
    if (ip == "::") {
        m_addr.ip6.sin6_family = AF_INET6;
        m_addr.ip6.sin6_addr = in6addr_any;
        m_addr.ip6.sin6_port = htons((uint16_t)port);
    } else {
        if (inet_pton(AF_INET6, ip.c_str(), &m_addr.ip6.sin6_addr) != 1) return false;
        m_addr.ip6.sin6_family = AF_INET6;
        m_addr.ip6.sin6_port = htons((uint16_t)port);
    } return true;
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
    if (addrs->ai_addrlen > sizeof(m_addr)) return false;
    memcpy(&m_addr, addrs->ai_addr, addrs->ai_addrlen);
    uint16_t temp = (uint16_t)htons((uint16_t)default_port);
    memcpy(m_addr.base.sa_data, &temp, sizeof(temp));
    freeaddrinfo(addrs);
    return true;
}
// -----------------------------------------------------------
bool SockAddr::is_multicast() const {
    switch (m_addr.base.sa_family) {
    case AF_INET:
        return ((unsigned char)m_addr.ip4.sin_port & 0xf0) == 0xe0;  // 224~239
    case AF_INET6:
        return ((unsigned char)m_addr.ip6.sin6_port == 0xff);  // high octet is 0xff
    } return false;
}
// -----------------------------------------------------------
uint32_t SockAddr::hash() const {
    uint32_t ret = 0;
    switch (m_addr.base.sa_family) {
    case AF_INET:
        memcpy(&ret, &m_addr.ip4.sin_addr, sizeof(ret));
        break;
    case AF_INET6:
        memcpy(&ret, &m_addr.ip6.sin6_addr, sizeof(ret));
        break;
    } return ret;
}
// -----------------------------------------------------------
}  // namespace lightcone
