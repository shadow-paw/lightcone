#ifndef LIGHTCONE_BEACON_H__
#define LIGHTCONE_BEACON_H__

#include <stdint.h>
#include <list>
#include <mutex>
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
    //! \param[in] mcast Broadcast destination
    //! \param[in] application_code Application defined code, this is used to filter beacon from different application.
    //! \param[in] service_code     Each application can have many services.
    //! \param[in] service_port     The port of such service.
    //! \param[in] interval         Beacon interval, in ms.
    //! \param[in] timeout          Timeout, in ms, for keep-alive.
    //! \return true on success, false on fail.
    bool init(const char* mcast,
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
    uint32_t myid() const { return m_service_id; }
    uint32_t myservice() const { return m_service_type; }
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
    static const uint64_t kNegotiateInterval = 3000;
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
    Udp      m_udp;
    SockAddr m_bcaddr;
    uint64_t m_beacon_timer, m_beacon_interval, m_beacon_timeout;
    // Service info
    uint32_t m_app, m_port;
    uint32_t m_service_type, m_service_id;
    uint32_t m_reserving_id, m_reserving_next, m_reserving_rand;
    int      m_collision;
    // Service table
    std::mutex              m_services_mutex;
    std::list<SERVICE_NODE> m_services;

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

