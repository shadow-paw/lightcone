#ifndef LIGHTCONE_SOCKADDR_H__
#define LIGHTCONE_SOCKADDR_H__

#include <stdint.h>
#include <string>
#include <utility>
#include "netinc.h"

namespace lightcone {
// -----------------------------------------------------------
//! Wrapper for sockaddr
class SockAddr {
friend class Socket;
friend class Udp;
 public:
    //! default constructor
    SockAddr() = default;
    ~SockAddr() = default;
    //! constructor with struct ip address
    //! \param[in] ip ipv4 with "12.34.56.78" format, or ipv6 with "0:0:0:0:0:0:0:1" format.
    //! \param[in] port port number, e.g. 80
    //! \exception std::invalid_argument()
    explicit SockAddr(const std::string& ip, int port);
    //! constructor with struct sockaddr
    //! \param[in] o sockaddr to copy
    explicit SockAddr(const struct sockaddr& o);
    //! constructor with struct sockaddr
    //! \param[in] o sockaddr to copy
    explicit SockAddr(const struct sockaddr_in& o);
    //! constructor with struct sockaddr
    //! \param[in] o sockaddr to copy
    explicit SockAddr(const struct sockaddr_in6& o);
    //! copy constructor
    //! \param[in] o Object to copy
    SockAddr(const SockAddr& o);
    //! move constructor
    //! \param[in] o Object to move
    SockAddr(SockAddr&& o);
    //! copy assignment
    //! \param[in] o Object to copy
    SockAddr& operator=(const SockAddr& o);
    //! move assignment
    //! \param[in] o Object to move
    SockAddr& operator=(SockAddr&& o);

    int get_domain() const { return m_addr.base.sa_family; }
    bool is_ip4() const { return m_addr.base.sa_family == AF_INET; }
    bool is_ip6() const { return m_addr.base.sa_family == AF_INET6; }
    std::pair<const struct sockaddr*, socklen_t> get_addr() const;

    uint32_t get_ip4() const;

    void set_port(int port);
    int get_port() const;
    //! Convert to printable string
    //! \return printable string, e.g. "12.34.56.78:1234"
    std::string to_string() const;
    //! Check if address is multicast
    //! \return true if multicast
    bool is_multicast() const;
    //! Set ipv4 or ipv6 address
    //! \param[in] ip ipv4 address, e.g. "12.34.56.78", "0:0:0:0:0:0:0:1"
    //! \param[in] port port number, e.g. 80
    //! \return true on success, false on fail with no side-effect.
    bool set_ip(const std::string& ip, int port);
    //! Set ipv4 address with inet_pton
    //! \param[in] ip ipv4 address, e.g. "12.34.56.78"
    //! \param[in] port port number, e.g. 80
    //! \return true on success, false on fail with no side-effect.
    bool set_ip4(const std::string& ip, int port);
    //! Set ipv6 address with inet_pton
    //! \param[in] ip ipv6 address, e.g. "0:0:0:0:0:0:0:1"
    //! \param[in] port port number, e.g. 80
    //! \return true on success, false on fail with no side-effect.
    bool set_ip6(const std::string& ip, int port);
    //! Resolve address in blocking mode.
    //! \param[in] host hostname with an optional port, e.g. "www.example.com:80"
    //! \param[in] port port number, e.g. 80
    //! \return true on success, false on fail with no side-effect.
    bool resolve(const std::string& host, int default_port);
    //! Return a hash based on remote address
    //! \return hash based on remote address
    uint32_t hash() const;

 private:
    union {
        struct sockaddr     base;
        struct sockaddr_in  ip4;
        struct sockaddr_in6 ip6;
    } m_addr;
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_SOCKADDR_H__
