#ifndef LIGHTCONE_NETENGINE_H__
#define LIGHTCONE_NETENGINE_H__

#include <stdint.h>
#include <mutex>
#include <functional>
#include "copyable.h"
#include "msgqueue.h"
#include "threads.h"
#include "netinc.h"
#include "tcp.h"
#include "loadbalancer.h"

namespace lightcone {
// -----------------------------------------------------------
//! Socket Engine
class NetEngine : private NonCopyable<NetEngine>, protected Threads {
 public:
    NetEngine() = delete;
    //! constructor
    //! \param[in] lb Choice of load balancer implementation, e.g. LoadBalancerRR
    explicit NetEngine(LoadBalancer<uint32_t>* lb);
    virtual ~NetEngine() = default;

    //! Start server
    //! \param[in] threads Number of threads, negative means scale factor with cpu core.
    //! \return true on success, false on fail with no side-effect.
    //! \sa stop
    bool start(int threads);
    //! Stop server
    //! \sa start
    void stop();
    //! Create a socket listen for a specific address
    //! \param[in] addr Address and port to listen for
    //! \param[in] initializer Initializer for newly created socket
    //! \return true on success, false on fail with no side-effect.
    //! \sa cb_tcpd_accepted
    bool listen(const SockAddr& addr, std::function<bool(Tcp*)> initializer = nullptr);
    //! Create a socket connect to a specific address
    //! \param[in] addr Address and port to connect to
    //! \param[in] initializer Initializer for newly created socket
    //! \return true on success, false on fail with no side-effect.
    //! \sa cb_tcpd_opened, cb_tcpd_refused
    bool connect(const SockAddr& addr, std::function<bool(Tcp*)> initializer = nullptr);
    //! Set socket timeout
    //! \param[in] timeout Fire timeout event if socket has no activity. In milliseconds. 0 to disable.
    void set_timeout(uint64_t timeout);

 protected:
    // delegate
    // -------------------------------------------------------
    virtual bool cb_net_timeout(Tcp* conn, uint64_t now) = 0;
    virtual bool cb_net_refused(Tcp* conn, uint64_t now) = 0;
    virtual bool cb_net_accepted(Tcp* conn, Tcp* from, uint64_t now) = 0;
    virtual bool cb_net_opened(Tcp* conn, uint64_t now) = 0;
    virtual bool cb_net_closed(Tcp* conn, uint64_t now) = 0;
    virtual bool cb_net_sent(Tcp* conn, uint64_t now) = 0;
    virtual bool cb_net_recv(Tcp* conn, uint64_t now) = 0;

 private:
    // Thread Handler
    // -------------------------------------------------------
    virtual void worker(unsigned int id, bool* runflag);
    void worker_listen(unsigned int id, bool* runflag);
    void worker_socket(unsigned int id, bool* runflag);

 private:
    static const int kMaxWorker = 128;
    std::mutex _mutex;
    LoadBalancer<uint32_t>* _lb;
    MessageQueue<Tcp*> _inbox[kMaxWorker+1];  // +1 for listen thread
    uint64_t _timeout;
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_NETENGINE_H__
