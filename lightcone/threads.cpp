#include "threads.h"

namespace lightcone {
// -----------------------------------------------------------
Threads::Threads() {
    _threads_count = 0;
}
Threads::~Threads() {
    stop();
}
void Threads::sleep(unsigned int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}
void Threads::msleep(unsigned int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}
void Threads::usleep(unsigned int usec) {
    std::this_thread::sleep_for(std::chrono::microseconds(usec));
}
bool Threads::start(unsigned int num) {
    if (num <= 0 || num > kMaxThreads) return false;
    _threads_count = num;
    for (unsigned int i=0; i < num; i++) {
        _threads[i].runflag = true;
        _threads[i].thread = std::thread(stub, this, i);
    } return true;
}
void Threads::stop() {
    for (unsigned int i=0; i < _threads_count; i++) {
        _threads[i].runflag = false;
    }
    for (unsigned int i=0; i < _threads_count; i++) {
        _threads[i].thread.join();
    }
    _threads_count = 0;
}
void Threads::stub(Threads* self, unsigned int id) {
    self->worker(id, &self->_threads[id].runflag);
}
// -----------------------------------------------------------
}  // namespace lightcone
