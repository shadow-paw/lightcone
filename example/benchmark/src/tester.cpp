#include <random>
#include "tester.h"

namespace example {
// -----------------------------------------------------------
Tester::Tester() : lightcone::NetEngine(&m_lb) {
    set_timeout(1000);
    m_testing = false;
    m_concurrent = 1;
    m_packet_size = 1;
    m_sendbuf = nullptr;
    connects = 0;
    packets = 0;
    timeouts = 0;
}
// -----------------------------------------------------------
Tester::~Tester() {
    delete m_sendbuf;
}
// -----------------------------------------------------------
bool Tester::setup(int concurrent, size_t packet_size) {
    unsigned char* sendbuf = new unsigned char[packet_size];
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (size_t i=0; i < packet_size; i++) {
        sendbuf[i] = (unsigned char)dis(rng);
    }
    delete m_sendbuf;
    m_sendbuf = sendbuf;
    m_concurrent = concurrent;
    m_packet_size = packet_size;
    return true;
}
// -----------------------------------------------------------
bool Tester::start(int threads) {
    m_testing = true;
    return lightcone::NetEngine::start(threads);
}
// -----------------------------------------------------------
void Tester::stop() {
    m_testing = false;
    lightcone::NetEngine::stop();
}
// -----------------------------------------------------------
bool Tester::cb_net_timeout(lightcone::Tcp* conn, uint64_t now) {
    timeouts++;
    return true;
}
// -----------------------------------------------------------
bool Tester::cb_net_refused(lightcone::Tcp* conn, uint64_t now) {
    if (m_testing) {
        lightcone::Threads::usleep(100);
        // attemp reconnect after slight delay
        connect(conn->get_remote());
    }
    return true;
}
// -----------------------------------------------------------
bool Tester::cb_net_accepted(lightcone::Tcp* conn, uint64_t now) {
    return true;
}
// -----------------------------------------------------------
bool Tester::cb_net_opened(lightcone::Tcp* conn, uint64_t now) {
    connects++;
    conn->send(m_sendbuf, m_packet_size);
    packets++;
    return true;
}
// -----------------------------------------------------------
bool Tester::cb_net_closed(lightcone::Tcp* conn, uint64_t now) {
    connects--;
    if (m_testing) {
        // attemp reconnect
        connect(conn->get_remote());
    } return true;
}
// -----------------------------------------------------------
bool Tester::cb_net_recv(lightcone::Tcp* conn, uint64_t now) {
    conn->recv([this, conn](const uint8_t* rbuf, size_t rlen) -> size_t {
        size_t consumed = 0;
        while (rlen >= m_packet_size) {
            conn->send(m_sendbuf, m_packet_size);
            packets += 2;  // one in, one out
            rlen -= m_packet_size;
            consumed += m_packet_size;
        }
        return consumed;
    });
    return true;
}
// -----------------------------------------------------------
bool Tester::cb_net_sent(lightcone::Tcp* conn, uint64_t now) {
    return true;
}
// -----------------------------------------------------------
}  // namespace example
