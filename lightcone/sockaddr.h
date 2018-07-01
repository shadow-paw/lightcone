#ifndef LIGHTCONE_SOCKADDR_H__
#define LIGHTCONE_SOCKADDR_H__

#include <stdint.h>
#include <string>
#include "netinc.h"

namespace lightcone {
// -----------------------------------------------------------
//! Wrapper for sockaddr
class SockAddr {
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

    // implicit cast to const sockaddr
    operator struct sockaddr& () { return m_addr; }
    operator const struct sockaddr& () const { return m_addr; }

    void set_port(int port);
    int get_port() const;
    uint32_t get_ip4() const;
    //! Convert to printable string
    //! \return printable string, e.g. "12.34.56.78:1234"
    std::string to_string() const;
    //! Check if address is multicast
    //! \return true if multicast
    bool is_multicast() const;
    //! Set ipv4 address with inet_pton
    //! \param[in] ip ipv4 address, e.g. "12.34.56.78"
    //! \param[in] port port number, e.g. 80
    //! \return true on success, false on fail with no side-effect.
    bool ip4(const std::string& ip, int port);
    //! Set ipv6 address with inet_pton
    //! \param[in] ip ipv6 address, e.g. "0:0:0:0:0:0:0:1"
    //! \param[in] port port number, e.g. 80
    //! \return true on success, false on fail with no side-effect.
    bool ip6(const std::string& ip, int port);
    //! Resolve address in blocking mode.
    //! \param[in] host hostname with an optional port, e.g. "www.example.com:80"
    //! \param[in] port port number, e.g. 80
    //! \return true on success, false on fail with no side-effect.
    bool resolve(const std::string& host, int default_port);
    //! Return a hash based on remote address
    //! \return hash based on remote address
    uint32_t hash() const;

 private:
    struct sockaddr m_addr;
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_SOCKADDR_H__
