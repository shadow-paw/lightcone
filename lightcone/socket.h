#ifndef LIGHTCONE_SOCKET_H__
#define LIGHTCONE_SOCKET_H__

#include <functional>
#include "netinc.h"
#include "copyable.h"
#include "sockaddr.h"

namespace lightcone {
// -----------------------------------------------------------
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    typedef SOCKET RAW_SOCKET;
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
    typedef int RAW_SOCKET;
#endif

//! Wrapper for socket
class Socket : private NonCopyable<Socket> {
 public:
    //! default constructor
    Socket();
    ~Socket();
    //! constructor with raw socket
    //! \param[in] o raw socket from BSD socket()
    explicit Socket(const RAW_SOCKET& o);
    //! move constructor
    //! \param[in] o Object to move
    Socket(Socket&& o);
    //! move assignment
    //! \param[in] o Object to move
    Socket& operator=(Socket&& o);

    // implicit cast to SOCKET
    operator RAW_SOCKET& () { return _fd; }
    operator const RAW_SOCKET& () const { return _fd; }

    //! Initialize socket
    //! \param[in] domain AF_INET or AF_INET6
    //! \param[in] type socket type as in bsd socket() function, e.g. SOCK_STREAM
    //! \param[in] proto socket protocol as in bsd socket() function, e.g. IPPROTO_TCP
    //! \return true on success, false on fail with no side-effect.
    bool init(int domain, int type, int proto);
    //! Check if socket is valid
    //! \return true if initialized, false otherwise.
    bool is_valid() const { return _fd != INVALID_SOCKET; }
    //! Check if socket error occured
    //! \return true if error occured
    bool is_error() const;
    //! wrapper to bind
    //! \param[in] addr Address and port to bind
    //! \return true on success, false on fail with no side-effect.
    bool bind(const SockAddr& addr);
    //! wrapper to listen
    //! \return true on success, false on fail with no side-effect.
    bool listen();
    //! wrapper to accept
    //! \param[out] addr Receive remote address. Maybe nullptr.
    //! \param[in] handler callback function when a socket is accepted, return false to reject.
    //! \return true on success, false on fail with no side-effect.
    bool accept(SockAddr* addr = nullptr, std::function<bool(Socket&& accepted, const SockAddr& addr)> handler = nullptr);
    //! wrapper to connect
    //! \param[in] addr Address and port to connect
    //! \return true on success, false on fail with no side-effect.
    bool connect(const SockAddr& addr);
    //! Close socket
    void close();
    //! Get local address, i.e. getsockname()
    //! \return local address
    SockAddr get_local() const;
    //! Get remote address, i.e. getpeername()
    //! \return remote address
    SockAddr get_remote() const;
    //! Set socket option
    //! \param[in] b true to set socket to non-blocking, false to blocking
    //! \return true on success, false on fail with no side-effect.
    bool set_nonblocking(bool b);
    //! Enable reuse address
    //! \return true on success, false on fail with no side-effect.
    bool set_reuse();

 private:
    RAW_SOCKET _fd;
#if defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
    const RAW_SOCKET INVALID_SOCKET = (RAW_SOCKET)-1;
#endif
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_SOCKET_H__
