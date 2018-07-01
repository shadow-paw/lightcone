#include "threads.h"

namespace lightcone {
// -----------------------------------------------------------
Threads::Threads() {
    m_threads_count = 0;
}
// -----------------------------------------------------------
Threads::~Threads() {
    stop();
}
// -----------------------------------------------------------
void Threads::sleep(unsigned int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}
// -----------------------------------------------------------
void Threads::msleep(unsigned int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}
// -----------------------------------------------------------
void Threads::usleep(unsigned int usec) {
    std::this_thread::sleep_for(std::chrono::microseconds(usec));
}
// -----------------------------------------------------------
bool Threads::start(unsigned int num) {
    if (num <= 0 || num > kMaxThreads) return false;
    m_threads_count = num;
    for (unsigned int i=0; i < num; i++) {
        m_threads[i].runflag = true;
        m_threads[i].thread = std::thread(stub, this, i);
    } return true;
}
// -----------------------------------------------------------
void Threads::stop() {
    for (unsigned int i=0; i < m_threads_count; i++) {
        m_threads[i].runflag = false;
    }
    for (unsigned int i=0; i < m_threads_count; i++) {
        m_threads[i].thread.join();
    }
    m_threads_count = 0;
}
// -----------------------------------------------------------
void Threads::stub(Threads* self, unsigned int id) {
    self->worker(id, &self->m_threads[id].runflag);
}
// -----------------------------------------------------------
}  // namespace lightcone
