#include <stdio.h>
#include <atomic>
#include "lightcone/lightcone.h"

// -----------------------------------------------------------
class TestThreads : public lightcone::Threads {
 public:
    std::atomic<int> spawned;
    std::atomic<int> check;
    lightcone::MessageQueue<int> queue;

    TestThreads() { spawned = 0; check = 0; }

 protected:
    void worker(unsigned int id, bool* runflag) {
        spawned++;
        while (*runflag || !queue.empty()) {
            int r = 0;
            if (!queue.get(&r, 10)) continue;
            // NOTE: We post incremental message from 1..1024
            if (check +1 == r) check = r;
        }
    }
};
// -----------------------------------------------------------
bool queue_access() {
    const int queue_size = 1024;
    lightcone::MessageQueue<int> queue;
    // fill queue, note the -1 because one slot is "reading"
    for (int i=0; i < queue_size; i++) {
        if (!queue.post(i)) return false;
    }
    // fetch queue
    for (int i=0; i < queue_size; i++) {
        int r = 0;
        if (!queue.get(&r, 0)) return false;
        if (r != i) return false;
    }
    // should be empty
    if (!queue.empty()) return false;
    return true;
}
// -----------------------------------------------------------
bool threads_spawn() {
    TestThreads threads;
    if (!threads.start(8)) return false;
    for (int i=0; ; i++) {
        if (threads.spawned == 8) break;
        lightcone::Threads::msleep(10);
        if (i >= 100 * 5) return false;  // 5sec
    }
    threads.stop();
    return true;
}
// -----------------------------------------------------------
bool threads_message() {
    TestThreads threads;
    if (!threads.start(1)) return false;
    for (int i=1; i <= 1024; i++) {
        threads.queue.post(i);
    }
    threads.stop();
    if (threads.check != 1024) return false;
    return true;
}
// -----------------------------------------------------------
bool run_tests() {
    if (!queue_access()) { printf ("FAILED. queue_access()\n"); return false; }
    if (!threads_spawn()) { printf ("FAILED. threads_spawn()\n"); return false; }
    if (!threads_message()) { printf ("FAILED. threads_message()\n"); return false; }
    return true;
}
// -----------------------------------------------------------
int main(int argc, char* argv[]) {
    bool success = run_tests();
    return success ? 0 : 1;
}
// -----------------------------------------------------------
