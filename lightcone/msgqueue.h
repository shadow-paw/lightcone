#ifndef LIGHTCONE_MSGQUEUE_H_
#define LIGHTCONE_MSGQUEUE_H_

#include <mutex>
#include <condition_variable>
#include <deque>
#include "copyable.h"

namespace lightcone {
// -----------------------------------------------------------
//! Message Queue
template <typename MSG>
class MessageQueue : private NonCopyable<MessageQueue<MSG>> {
 public:
    //! default constructor
    MessageQueue() = default;
    ~MessageQueue() = default;
    //! check if queue is empty
    //! \return true if empty
    bool empty() const;
    //! Post a message
    //! \param msg Message to post into queue
    //! \return true on success, false on fail with no side-effect.
    bool post(const MSG& msg);
    //! Post a message
    //! \param msg Message to post into queue
    //! \return true on success, false on fail with no side-effect.
    bool post(MSG&& msg);
    //! get a message
    //! \param msg Receive message from queue
    //! \param milliseconds Timeout value, in milliseconds, 0 without waiting
    //! \return true on success, false on fail with no side-effect.
    bool get(MSG* msg, int milliseconds);

 private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::deque<MSG> m_queue;
};
// -----------------------------------------------------------
// Implementation
// -----------------------------------------------------------
template <typename MSG>
bool MessageQueue<MSG>::empty() const {
    return m_queue.empty();
}
// -----------------------------------------------------------
template <typename MSG>
bool MessageQueue<MSG>::post(const MSG& msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push_back(msg);
    return true;
}
// -----------------------------------------------------------
template <typename MSG>
bool MessageQueue<MSG>::post(MSG&& msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push_back(msg);
    return true;
}
// -----------------------------------------------------------
template <typename MSG>
bool MessageQueue<MSG>::get(MSG* msg, int milliseconds) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_queue.empty()) {
        m_cond.wait_for(lock, std::chrono::microseconds(milliseconds));
        if (m_queue.empty()) return false;
    }
    *msg = m_queue.front();
    m_queue.pop_front();
    return true;
}
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_MSGQUEUE_H_
