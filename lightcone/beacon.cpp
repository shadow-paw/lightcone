#include <random>
#include "beacon.h"
#include "byteorder.h"
#include "calendar.h"

#define CMD_BEACON (0)
#define CMD_RESERVEID (1)

namespace lightcone {
// -----------------------------------------------------------
Beacon::Beacon() {
    _app = 0;
    _service_type = 0;
    _service_id = 0;
    _reserving_id = 0;
    _reserving_next = 0;
    _collided = false;
    _port = 0;
    _beacon_interval = _beacon_timeout = 1000;
    _beacon_timer = 0;
}
bool Beacon::init(const SockAddr& mcast_addr,
                  uint32_t app, uint32_t service_type, uint32_t service_port,
                  uint64_t interval, uint64_t timeout) {
    _app             = app;
    _service_type    = service_type;
    _service_id      = 0;
    _reserving_id    = 0;
    _reserving_next  = 0;
    _port            = service_port;
    _beacon_interval = interval;
    _beacon_timeout  = timeout;
    _beacon_timer    = 0;
    _collided        = true;
    _bcaddr = mcast_addr;
    SockAddr inaddr;
    if (mcast_addr.is_ip4()) {
        inaddr.set_ip4("0.0.0.0", _bcaddr.get_port());
    } else if (mcast_addr.is_ip6()) {
        inaddr.set_ip6("::", _bcaddr.get_port());
    } else {
        return false;
    }
    if (!_udp.open(inaddr.get_family(), true)) return false;
    if (!_udp.bind(inaddr, true)) return false;
    if (!_udp.joinmcast(mcast_addr)) return false;
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
    if (_service_id == 0) return false;
    packet[0] = ByteOrder::htoel(kMagic);
    packet[1] = ByteOrder::htoel(_app);
    packet[2] = ByteOrder::htoel((uint32_t)CMD_BEACON);
    packet[3] = ByteOrder::htoel(_service_type);
    packet[4] = ByteOrder::htoel(_service_id);
    packet[5] = ByteOrder::htoel(_port);
    _udp.send(_bcaddr, packet, sizeof(packet));
    return true;
}
bool Beacon::send_reserveid(uint32_t id) {
    uint32_t packet[7];
    if (id == 0) return false;
    packet[0] = ByteOrder::htoel(kMagic);
    packet[1] = ByteOrder::htoel(_app);
    packet[2] = ByteOrder::htoel((uint32_t)CMD_RESERVEID);
    packet[3] = ByteOrder::htoel(_service_type);
    packet[4] = ByteOrder::htoel(id);
    packet[5] = ByteOrder::htoel(_port);
    packet[6] = ByteOrder::htoel(_reserving_rand);
    _udp.send(_bcaddr, packet, sizeof(packet));
    return true;
}
bool Beacon::service_update(const SockAddr& addr, uint32_t type, uint32_t id) {
    auto now = Calendar::now();
    uint32_t ip = addr.get_ip4();
    int port = addr.get_port();
    _services_mutex.lock();
    for (auto it=_services.begin(); it != _services.end(); ++it) {
        if (it->type != type || it->id != id) continue;
        if (it->addr.get_ip4() != ip || it->addr.get_port() != port) continue;
        it->keepalive_timer = now;
        _services_mutex.unlock();
        return true;
    }
    // Not found, add record
    _services.push_back({type, id, now, addr});
    _services_mutex.unlock();
    cb_beacon_peerup(type, id, addr);
    return true;
}
bool Beacon::service_remove(const SockAddr& addr) {
    uint32_t ip = addr.get_ip4();
    int port = addr.get_port();
    _services_mutex.lock();
    for (auto it=_services.begin(); it != _services.end(); ++it) {
        if (it->addr.get_ip4() != ip || it->addr.get_port() != port) continue;
        uint32_t type = it->type;
        uint32_t id   = it->id;
        _services.erase(it);
        _services_mutex.unlock();
        cb_beacon_peerdown(type, id, addr);
        return true;
    }
    _services_mutex.unlock();
    return false;
}
bool Beacon::service_healthcheck() {
    auto now = Calendar::now();
    _services_mutex.lock();
    for (auto it=_services.begin(); it != _services.end(); ) {
        if (now - it->keepalive_timer < _beacon_timeout) {
            ++it;
        } else {
            uint32_t type = it->type;
            uint32_t id   = it->id;
            SockAddr addr(it->addr);
            _services.erase(it++);
            _services_mutex.unlock();
            cb_beacon_peerdown(type, id, addr);
            _services_mutex.lock();
            it = _services.begin();
        }
    }
    _services_mutex.unlock();
    return true;
}
void Beacon::worker(unsigned int id, bool* runflag) {
    while (*runflag) {
        auto now = Calendar::now();
        if (_service_id == 0) {
            // No ID yet, negotiate
            if (now - _beacon_timer >= kNegotiateInterval) {
                _beacon_timer = now;
                if (_collided) {
                    // _reserving_id = (_service_type<<8) | (0xff & _reserving_next);
                    _reserving_id = cb_beacon_createid(_service_type, _reserving_next);
                    // some random numbers
                    std::random_device rd;
                    std::mt19937 rng(rd());
                    std::uniform_int_distribution<> dis(1, 0x7fffffff);
                    _reserving_next += (dis(rng) % 4) + 1;
                    _reserving_rand = (uint32_t)dis(rng);
                    _collided = false;
                    send_reserveid(_reserving_id);
                } else {
                    // no collision within kNegotiateInterval, use the id
                    _service_id = _reserving_id;
                    _reserving_id = 0;
                    send_beacon();
                    cb_beacon_ready(_service_type, _service_id);
                }
            }
        } else {
            if (now - _beacon_timer >= _beacon_interval) {
                _beacon_timer = now;
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

    if ((rlen = _udp.recv(&peer_addr, rbuf, sizeof(rbuf), 500)) <= 0) return false;
    if (rlen < 24) return true;

    if (rbuf[0] != ByteOrder::htoel(kMagic)) return true;
    if (rbuf[1] != ByteOrder::htoel(_app)) return true;
    cmd  = ByteOrder::eltoh(rbuf[2]);
    type = ByteOrder::eltoh(rbuf[3]);
    id   = ByteOrder::eltoh(rbuf[4]);
    port = ByteOrder::eltoh(rbuf[5]);
    r = ByteOrder::eltoh(rbuf[6]);

    switch (cmd) {
    case CMD_BEACON:
        if (id == _reserving_id) {
            _collided = true;
            _beacon_timer = 0;
        }
        if (id != _service_id) {
            peer_addr.set_port(static_cast<int>(port));
            service_update(peer_addr, type, id);
        }
        break;
    case CMD_RESERVEID:
        if (id == _reserving_id && r != _reserving_rand) {
            _collided = true;
            _beacon_timer = 0;
        }
        if (id == _service_id) {
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
