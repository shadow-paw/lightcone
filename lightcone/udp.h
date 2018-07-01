#ifndef LIGHTCONE_UDP_H__
#define LIGHTCONE_UDP_H__

#include <string>
#include "copyable.h"
#include "netinc.h"
#include "socket.h"

namespace lightcone {
// -----------------------------------------------------------
//! UDP Socket
class Udp : private NonCopyable<Udp> {
 public:
    //! default constructor
    Udp() = default;
    ~Udp() = default;
    //! move constructor
    //! \param[in] o Object to move
    Udp(Udp&& o);
    //! move assignment
    //! \param[in] o Object to move
    Udp& operator=(Udp&& o);

    //! Initialize with socket()
    //! \param[in] nonblocking Make socket as non-blocking
    //! \return true on success, false on fail with no side-effect.
    bool open(bool nonblocking);
    //! Close the socket.
    void close();
    //! wrapper to bind
    //! \param[in] addr Address and port to bind
    //! \param[in] reuse true to allow listen to same port
    //! \return true on success, false on fail with no side-effect.
    bool bind(const SockAddr& addr, bool reuse);
    //! wrapper to bind
    //! \param[in] addr Address to bind
    //! \param[in] port Pport to bind
    //! \param[in] reuse true to allow listen to same port
    //! \return true on success, false on fail with no side-effect.
    bool bind(const std::string& addr, int port, bool reuse);
    //! wrapper to send
    //! \param[in] addr Address and port to bind
    //! \param[in] buf Data to send
    //! \param[in] len Size of data, in bytes
    //! \return number of byte sent, negative value on failure.
    ssize_t send(const SockAddr& addr, const void* buf, size_t len);
    //! wrapper to recv
    //! \param[out] sender Address and port for sender
    //! \param[out] buf Data received
    //! \param[in] len Max size of data, in bytes
    //! \param[in] timeout to wait for incoming data, in milliseconds
    //! \return number of byte received, negative value on failure.
    ssize_t recv(SockAddr* sender, void* buf, size_t len, unsigned int timeout = 0);
    //! join multicast group
    //! \param[in] addr Address and port to join
    bool joinmcast(const SockAddr& addr);

 private:
    Socket m_socket;
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_UDP_H__
