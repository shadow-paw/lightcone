#include <random>
#include "beacon.h"
#include "byteorder.h"
#include "calendar.h"

#define CMD_BEACON (0)
#define CMD_RESERVEID (1)

namespace lightcone {
// -----------------------------------------------------------
Beacon::Beacon() {
    m_app = 0;
    m_service_type = 0;
    m_service_id = 0;
    m_reserving_id = 0;
    m_reserving_next = 0;
    m_collided = false;
    m_port = 0;
    m_beacon_interval = m_beacon_timeout = 1000;
    m_beacon_timer = 0;
}
bool Beacon::init(const SockAddr& mcast_addr,
                  uint32_t app, uint32_t service_type, uint32_t service_port,
                  uint64_t interval, uint64_t timeout) {
    m_app             = app;
    m_service_type    = service_type;
    m_service_id      = 0;
    m_reserving_id    = 0;
    m_reserving_next  = 0;
    m_port            = service_port;
    m_beacon_interval = interval;
    m_beacon_timeout  = timeout;
    m_beacon_timer    = 0;
    m_collided        = true;
    m_bcaddr = mcast_addr;
    SockAddr inaddr;
    if (mcast_addr.is_ip4()) {
        inaddr.set_ip4("0.0.0.0", m_bcaddr.get_port());
    } else if (mcast_addr.is_ip6()) {
        inaddr.set_ip6("::", m_bcaddr.get_port());
    } else {
        return false;
    }
    if (!m_udp.open(inaddr.get_domain(), true)) return false;
    if (!m_udp.bind(inaddr, true)) return false;
    if (!m_udp.joinmcast(mcast_addr)) return false;
    return true;
}
bool Beacon::start() {
    return Threads::start(1);
}
void Beacon::stop() {
    Threads::stop();
}
bool Beacon::send_beacon(void) {
    uint32_t packet[6];
    if (m_service_id == 0) return false;
    packet[0] = ByteOrder::htoel(kMagic);
    packet[1] = ByteOrder::htoel(m_app);
    packet[2] = ByteOrder::htoel((uint32_t)CMD_BEACON);
    packet[3] = ByteOrder::htoel(m_service_type);
    packet[4] = ByteOrder::htoel(m_service_id);
    packet[5] = ByteOrder::htoel(m_port);
    m_udp.send(m_bcaddr, packet, sizeof(packet));
    return true;
}
bool Beacon::send_reserveid(uint32_t id) {
    uint32_t packet[7];
    if (id == 0) return false;
    packet[0] = ByteOrder::htoel(kMagic);
    packet[1] = ByteOrder::htoel(m_app);
    packet[2] = ByteOrder::htoel((uint32_t)CMD_RESERVEID);
    packet[3] = ByteOrder::htoel(m_service_type);
    packet[4] = ByteOrder::htoel(id);
    packet[5] = ByteOrder::htoel(m_port);
    packet[6] = ByteOrder::htoel(m_reserving_rand);
    m_udp.send(m_bcaddr, packet, sizeof(packet));
    return true;
}
bool Beacon::service_update(const SockAddr& addr, uint32_t type, uint32_t id) {
    auto now = Calendar::now();
    uint32_t ip = addr.get_ip4();
    int port = addr.get_port();
    m_services_mutex.lock();
    for (auto it=m_services.begin(); it != m_services.end(); ++it) {
        if (it->type != type || it->id != id) continue;
        if (it->addr.get_ip4() != ip || it->addr.get_port() != port) continue;
        it->keepalive_timer = now;
        m_services_mutex.unlock();
        return true;
    }
    // Not found, add record
    m_services.push_back({type, id, now, addr});
    m_services_mutex.unlock();
    cb_beacon_peerup(type, id, addr);
    return true;
}
bool Beacon::service_remove(const SockAddr& addr) {
    uint32_t ip = addr.get_ip4();
    int port = addr.get_port();
    m_services_mutex.lock();
    for (auto it=m_services.begin(); it != m_services.end(); ++it) {
        if (it->addr.get_ip4() != ip || it->addr.get_port() != port) continue;
        uint32_t type = it->type;
        uint32_t id   = it->id;
        m_services.erase(it);
        m_services_mutex.unlock();
        cb_beacon_peerdown(type, id, addr);
        return true;
    }
    m_services_mutex.unlock();
    return false;
}
bool Beacon::service_healthcheck() {
    auto now = Calendar::now();
    m_services_mutex.lock();
    for (auto it=m_services.begin(); it != m_services.end(); ) {
        if (now - it->keepalive_timer < m_beacon_timeout) {
            ++it;
        } else {
            uint32_t type = it->type;
            uint32_t id   = it->id;
            SockAddr addr(it->addr);
            m_services.erase(it++);
            m_services_mutex.unlock();
            cb_beacon_peerdown(type, id, addr);
            m_services_mutex.lock();
            it = m_services.begin();
        }
    }
    m_services_mutex.unlock();
    return true;
}
void Beacon::worker(unsigned int id, bool* runflag) {
    while (*runflag) {
        auto now = Calendar::now();
        if (m_service_id == 0) {
            // No ID yet, negotiate
            if (now - m_beacon_timer >= kNegotiateInterval) {
                m_beacon_timer = now;
                if (m_collided) {
                    // m_reserving_id = (m_service_type<<8) | (0xff & m_reserving_next);
                    m_reserving_id = cb_beacon_createid(m_service_type, m_reserving_next);
                    // some random numbers
                    std::random_device rd;
                    std::mt19937 rng(rd());
                    std::uniform_int_distribution<> dis(1, 0x7fffffff);
                    m_reserving_next += (dis(rng) % 4) + 1;
                    m_reserving_rand = (uint32_t)dis(rng);
                    m_collided = false;
                    send_reserveid(m_reserving_id);
                } else {
                    // no collision within kNegotiateInterval, use the id
                    m_service_id = m_reserving_id;
                    m_reserving_id = 0;
                    send_beacon();
                    cb_beacon_ready(m_service_type, m_service_id);
                }
            }
        } else {
            if (now - m_beacon_timer >= m_beacon_interval) {
                m_beacon_timer = now;
                send_beacon();
            }
        }
        service_healthcheck();
        while (cb_recv()) {}
    }
}
bool Beacon::cb_recv() {
    SockAddr peer_addr;
    uint32_t rbuf[64] = { 0 };
    ssize_t  rlen;
    uint32_t cmd, type, id, port, r;

    if ((rlen = m_udp.recv(&peer_addr, rbuf, sizeof(rbuf), 500)) <= 0) return false;
    if (rlen < 24) return true;

    if (rbuf[0] != ByteOrder::htoel(kMagic)) return true;
    if (rbuf[1] != ByteOrder::htoel(m_app)) return true;
    cmd  = ByteOrder::eltoh(rbuf[2]);
    type = ByteOrder::eltoh(rbuf[3]);
    id   = ByteOrder::eltoh(rbuf[4]);
    port = ByteOrder::eltoh(rbuf[5]);
    r = ByteOrder::eltoh(rbuf[6]);

    switch (cmd) {
    case CMD_BEACON:
        if (id == m_reserving_id) {
            m_collided = true;
            m_beacon_timer = 0;
        }
        if (id != m_service_id) {
            peer_addr.set_port(static_cast<int>(port));
            service_update(peer_addr, type, id);
        }
        break;
    case CMD_RESERVEID:
        if (id == m_reserving_id && r != m_reserving_rand) {
            m_collided = true;
            m_beacon_timer = 0;
        }
        if (id == m_service_id) {
            send_beacon();
        }
        break;
    }
    return true;
}
bool Beacon::remove_peer(const SockAddr& addr) {
    return service_remove(addr);
}
// -----------------------------------------------------------
}  // namespace lightcone
