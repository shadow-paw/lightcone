#ifndef LIGHTCONE_NETPOLLER_H__
#define LIGHTCONE_NETPOLLER_H__

#include <list>
#include "copyable.h"
#include "netinc.h"
#include "socket.h"

// Polling method
#define SIGNAL_POLL_SELECT 0
#define SIGNAL_POLL_EPOLL  1
#define SIGNAL_POLL_KQUEUE 2

#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    #define SIGNAL_POLL_IMPLEMENTATION SIGNAL_POLL_SELECT
#elif defined(PLATFORM_BSD) || defined(PLATFORM_OSX) || defined(PLATFORM_IOS)
    #define SIGNAL_POLL_IMPLEMENTATION SIGNAL_POLL_KQUEUE
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
    #define SIGNAL_POLL_IMPLEMENTATION SIGNAL_POLL_EPOLL
#elif defined(PLATFORM_SOLARIS)
    #define SIGNAL_POLL_IMPLEMENTATION SIGNAL_POLL_SELECT
#else
    #error Not Implemented!
#endif

namespace lightcone {
// -----------------------------------------------------------
//! Poll for events with select/epoll/kqueue
class NetPoller : private NonCopyable<NetPoller> {
 public:
    static const int kClose = 1<<0;
    static const int kRead = 1<<1;
    static const int kWrite = 1<<2;
    static const int kReadWrite = kRead | kWrite;

    //! default constructor
    NetPoller();
    ~NetPoller();
    //! initialize poller
    //! \return true if success
    //! \sa fini
    bool init();
    //! cleanup poller. registered socket will not be closed automatically.
    //! \sa init
    void fini();
    //! Register socket into poller
    //! \param[in] fd raw socket
    //! \param[in] event Interested event: kRead, kWrite or kReadWrite
    //! \param[in] ud User-defined data
    //! \return true on success, false on fail with no side-effect.
    bool add(const RAW_SOCKET& fd, int event, void* ud);
    //! Unregister socket
    //! \param[in] fd raw socket
    //! \return true on success, false on fail with no side-effect.
    bool remove(const RAW_SOCKET& fd);
    //! Modify registration
    //! \param[in] fd raw socket
    //! \param[in] event Interested event: kRead, kWrite or kReadWrite
    //! \param[in] ud User-defined data
    //! \return true on success, false on fail with no side-effect.
    bool modify(const RAW_SOCKET& fd, int event, void* ud);
    //! Poll for socket event
    //! \param[in] milliseconds maximum wait time
    //! \param[in] cb Callback when a registered socket has event occured
    //! \return true when some event happened.
    bool poll(unsigned int milliseconds, std::function<bool(const RAW_SOCKET& fd, int event, void* ud)> cb);

 private:
#if SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_SELECT
    struct SelectItem {
        RAW_SOCKET fd;
        int event;
        void* ud;
    };
    std::list<SelectItem> m_list;
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_EPOLL
    static const size_t kEventCapacity = 1024;
    int m_queue;
    struct epoll_event* m_events;
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_KQUEUE
    static const size_t kEventCapacity = 1024;
    int m_queue;
    struct kevent* m_events;
    struct kevent* m_changes;
    int m_changes_count, m_changes_capacity;
    bool ensure_changes_size(int size);
#endif
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_NETPOLLER_H__
