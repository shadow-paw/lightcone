#include <random>
#include "tester.h"

namespace example {
// -----------------------------------------------------------
Tester::Tester() : lightcone::NetEngine(&_lb) {
    set_timeout(1000);
    _testing = false;
    _concurrent = 1;
    _packet_size = 1;
    _sendbuf = nullptr;
    connects = 0;
    packets = 0;
    timeouts = 0;
}
Tester::~Tester() {
    delete _sendbuf;
}
bool Tester::setup(int concurrent, size_t packet_size) {
    unsigned char* sendbuf = new unsigned char[packet_size];
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (size_t i=0; i < packet_size; i++) {
        sendbuf[i] = (unsigned char)dis(rng);
    }
    delete _sendbuf;
    _sendbuf = sendbuf;
    _concurrent = concurrent;
    _packet_size = packet_size;
    return true;
}
bool Tester::start(int threads) {
    _testing = true;
    return lightcone::NetEngine::start(threads);
}
void Tester::stop() {
    _testing = false;
    lightcone::NetEngine::stop();
}
bool Tester::cb_net_timeout(lightcone::Tcp* conn, uint64_t now) {
    timeouts++;
    return false;
}
bool Tester::cb_net_refused(lightcone::Tcp* conn, uint64_t now) {
    if (_testing) {
        lightcone::Threads::usleep(100);
        // attemp reconnect after slight delay
        connect(conn->get_remote());
    }
    return true;
}
bool Tester::cb_net_accepted(lightcone::Tcp* conn, lightcone::Tcp* from, uint64_t now) {
    return true;
}
bool Tester::cb_net_opened(lightcone::Tcp* conn, uint64_t now) {
    connects++;
    conn->send(_sendbuf, _packet_size);
    packets++;
    return true;
}
bool Tester::cb_net_closed(lightcone::Tcp* conn, uint64_t now) {
    connects--;
    if (_testing) {
        // attemp reconnect
        connect(conn->get_remote());
    } return true;
}
bool Tester::cb_net_recv(lightcone::Tcp* conn, uint64_t now) {
    conn->recv([this, conn](const uint8_t* rbuf, size_t rlen) -> size_t {
        size_t consumed = 0;
        while (rlen >= _packet_size) {
            conn->send(_sendbuf, _packet_size);
            packets += 2;  // one in, one out
            rlen -= _packet_size;
            consumed += _packet_size;
        }
        return consumed;
    });
    return true;
}
bool Tester::cb_net_sent(lightcone::Tcp* conn, uint64_t now) {
    return true;
}
// -----------------------------------------------------------
}  // namespace example
