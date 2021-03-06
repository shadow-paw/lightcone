#ifndef LIGHTCONE_BEACON_H__
#define LIGHTCONE_BEACON_H__

#include <stdint.h>
#include <list>
#include <mutex>
#include <string>
#include "copyable.h"
#include "udp.h"
#include "sockaddr.h"
#include "threads.h"

namespace lightcone {
// -----------------------------------------------------------
//! Beacon daemon like the Bonjour
class Beacon : private NonCopyable<Beacon>, private Threads {
 public:
    Beacon();
    virtual ~Beacon() = default;

    //! Setup beacon parameters
    //! \param[in] mcast_addr multicast address and port, e.g. 239.0.0.1, FF05:0:0:0:0:0:0:2
    //! \param[in] application_code Application defined code, this is used to filter beacon from different application.
    //! \param[in] service_code     Each application can have many services.
    //! \param[in] service_port     The port of such service.
    //! \param[in] interval         Beacon interval, in ms.
    //! \param[in] timeout          Timeout, in ms, for keep-alive.
    //! \return true on success, false on fail.
    bool init(const SockAddr& mcast_addr,
             uint32_t app, uint32_t service_type, uint32_t service_port,
             uint64_t interval, uint64_t timeout);
    //! Start server
    //! \return true on success, false on fail.
    bool start();
    //! Stop server
    void stop();

 protected:
    // Protected Functions
    // -------------------------------------------------------
    uint32_t myid() const { return _service_id; }
    uint32_t myservice() const { return _service_type; }
    bool remove_peer(const SockAddr& addr);

    // Interface
    // -------------------------------------------------------
    virtual uint32_t cb_beacon_createid(uint32_t type, uint32_t index) = 0;
    virtual bool     cb_beacon_ready(uint32_t type, uint32_t id) = 0;
    virtual bool     cb_beacon_peerup(uint32_t type, uint32_t id, const SockAddr& addr) = 0;
    virtual bool     cb_beacon_peerdown(uint32_t type, uint32_t id, const SockAddr& addr) = 0;

 private:
    // Thread Handler
    // -------------------------------------------------------
    void worker(unsigned int id, bool* runflag);

 private:
    // Internal Constants
    // -------------------------------------------------------
    static const uint64_t kNegotiateInterval = 1000;
    static const uint32_t kMagic = 0xbadc0de7;

 private:
    // Internal Data Area
    // -------------------------------------------------------
    struct SERVICE_NODE {
        uint32_t type;
        uint32_t id;
        uint64_t keepalive_timer;
        SockAddr addr;
    };
    // -------------------------------------------------------
    Udp      _udp;
    SockAddr _bcaddr;
    uint64_t _beacon_timer, _beacon_interval, _beacon_timeout;
    // Service info
    uint32_t _app, _port;
    uint32_t _service_type, _service_id;
    uint32_t _reserving_id, _reserving_next, _reserving_rand;
    bool     _collided;
    // Service table
    std::mutex              _services_mutex;
    std::list<SERVICE_NODE> _services;

 private:
    // Internal Helper
    // -------------------------------------------------------
    bool service_update(const SockAddr& addr, uint32_t service, uint32_t id);
    bool service_remove(const SockAddr& addr);
    bool service_healthcheck();
    bool send_beacon();
    bool send_reserveid(uint32_t id);
    bool cb_recv();
};

// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_BEACON_H__
