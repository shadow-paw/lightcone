#ifndef LIGHTCONE_THREADS_H__
#define LIGHTCONE_THREADS_H__

#include <thread>
#include "copyable.h"

namespace lightcone {
// -----------------------------------------------------------
//! Thread Pool
class Threads : private NonCopyable<Threads> {
 public:
    Threads();
    virtual ~Threads();

    //! Get number of threads
    //! \return number of threads in this pool
    unsigned int thread_count() const { return _threads_count; }
    //! Start threads
    //! \param[in] num Number of threads, num >= 1
    //! \return true on success, false on fail with no side-effect.
    //! \sa stop
    bool start(unsigned int num);
    //! Stop all threads, block until all threads are stopped
    //! \return true on success, false on fail with no side-effect.
    //! \sa start
    void stop();
    //! Sleep current thread
    //! \param[in] seconds Number of seconds
    //! \sa msleep, usleep
    static void sleep(unsigned int seconds);
    //! Sleep current thread
    //! \param[in] milliseconds Number of milliseconds
    //! \sa sleep, usleep
    static void msleep(unsigned int milliseconds);
    //! Sleep current thread
    //! \param[in] usec Number of microseconds
    //! \sa sleep, msleep
    static void usleep(unsigned int usec);

 protected:
    //! Thread worker. Thread stop upon return.
    //! \param[in] id identity of the thread which run the worker
    //! \param[in] runflag Reference to a bool indicate true if the thread interrupted by stop()
    virtual void worker(unsigned int id, bool* runflag) = 0;

 private:
    static const size_t kMaxThreads = 128;
    struct THREAD_INFO {
        bool        runflag;
        std::thread thread;
    };
    THREAD_INFO  _threads[kMaxThreads];
    unsigned int _threads_count;

 private:
    //! \cond Skip this from doxygen
    //! Internal thread stub
    static void stub(Threads* self, unsigned int id);
    //! \endcond
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_THREADS_H__
