#ifndef LIGHTCONE_TCP_H__
#define LIGHTCONE_TCP_H__

#include <functional>
#include <string>
#include <unordered_map>
#include "buffer.h"
#include "copyable.h"
#include "netinc.h"
#include "socket.h"

namespace lightcone {
// -----------------------------------------------------------
//! TCP Socket
class Tcp : private NonCopyable<Tcp> {
friend class NetEngine;
 public:
    //! convenient holder for user-defined data
    std::unordered_map<uint32_t, uintptr_t> ud;
    //! convenient holder for user-defined protocol indicator
    int protocol;

    //! default constructor
    Tcp();
    ~Tcp();
    //! move constructor
    //! \param[in] o Object to move
    Tcp(Tcp&& o);
    //! move assignment
    //! \param[in] o Object to move
    Tcp& operator=(Tcp&& o);

    //! Set socket option
    //! \param[in] b true to set socket to non-blocking, false to blocking
    //! \return true on success, false on fail with no side-effect.
    bool set_nonblocking(bool b);
    //! Get local address, i.e. getsockname()
    //! \return local address
    SockAddr get_local() const;
    //! Get remote address, i.e. getpeername()
    //! \return remote address
    SockAddr get_remote() const;
    //! Listen to the specified address.
    //! \param[in] addr Address to listen
    //! \param[in] reuse true to allow listen to same port
    //! \return true on success, false on fail and reset to closed state.
    //! \sa accept
    bool listen(const SockAddr& addr, bool reuse);
    //! Accept connection. This Tcp should have called listen().
    //! \param[out] addr Receive remote address. Maybe nullptr.
    //! \param[in] firewall callback to check against remote address, return false to reject connection. Maybe nullptr.
    //! \return Pointer to newly created socket, nullptr on fail.
    //! \sa listen
    Tcp* accept(SockAddr* addr = nullptr, std::function<bool(const SockAddr& addr)> firewall = nullptr);
    //! Connect to the specified address.
    //! \param[in] addr Address to connect
    //! \param[in] nonblocking Non-blocking connect
    //! \param[in] cb Callback upon connected
    //! \return true on success, false on fail and reset to closed state.
    bool connect(const SockAddr& addr, bool nonblocking = false, std::function<void(bool success)> cb = nullptr);
    //! Connect to the specified address.
    //! \param[in] hostname Hostname to connect, e.g. "www.example.com"
    //! \param[in] port Port to connect
    //! \param[in] nonblocking Non-blocking connect
    //! \param[in] cb Callback upon connected
    //! \return true on success, false on fail and reset to closed state.
    bool connect(const std::string& addr, int port, bool nonblocking = false, std::function<void(bool success)> cb = nullptr);
    //! Close the socket.
    void close();
    //! Send data
    //! \param[in] data data to send
    //! \return true on success, false on fail with no side-effect.
    bool send(const std::string& data);
    //! Send data
    //! \param[in] data data to send
    //! \param[in] datalen length, in bytes
    //! \return true on success, false on fail with no side-effect.
    bool send(const void* data, size_t datalen);
    //! Send data
    //! \param[in] reserve_size Length of memory to reserve for writing.
    //! \param[in] cb Callback to prepare send buffer, return number of bytes to commit.
    //! \return true on success, false on fail with no side-effect.
    bool send(size_t reserve_size, std::function<ssize_t(uint8_t* wbuf, size_t wlen)> cb);
    //! Receive data
    //! \param[in] cb Callback to receive data, return number of bytes processed.
    //! \return true on success, false on fail with no side-effect.
    bool recv(std::function<size_t(const uint8_t* rbuf, size_t rlen)> cb);
    //! Poll for io event
    //! \param[in] milliseconds Timeout, in milliseconds.
    //! \return 0 on no event or managed by engine, 1 if io happened, -1 if error
    int poll(unsigned int milliseconds);

 private:
    bool io_read();
    bool io_write();

 private:
    Socket m_socket;
    Buffer m_ibuf, m_obuf;
    std::function<void(bool success)> m_cb_connect;

 private:  // used by engine
    //! socket state
    enum State {
        Uninitalized,
        Listening,
        Accepted,
        Connnecting,
        Connected,
        Refused,
        Closing,
        Closed
    };
    bool m_managed;
    uint32_t m_lb_hash;
    State m_state;
    int m_evmask;
    uint64_t m_atime;
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_TCP_H__
