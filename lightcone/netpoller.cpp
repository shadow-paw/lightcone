#include "netpoller.h"

using lightcone::NetPoller;

// -----------------------------------------------------------
NetPoller::NetPoller() {
#if SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_SELECT
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_EPOLL
    m_queue = 0;
    m_events = nullptr;
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_KQUEUE
    m_queue = 0;
    m_events = nullptr;
    m_changes = nullptr;
    m_changes_count = m_changes_capacity = 0;
#else
    #error Not Implemented!
#endif
}
// -----------------------------------------------------------
NetPoller::~NetPoller() {
    fini();
}
// -----------------------------------------------------------
#if SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_KQUEUE
bool NetPoller::ensure_changes_size(int size) {
    if ( size > m_changes_capacity ) {
        struct kevent* p = (struct kevent*)realloc(m_changes, ((size_t)size+16)*sizeof(struct kevent));
        if (!p) return false;
        m_changes = p;
        m_changes_capacity = size+16;
    } return true;
}
#endif
// -----------------------------------------------------------
bool NetPoller::init() {
#if SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_SELECT
    return true;
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_EPOLL
    m_events = (struct epoll_event*)malloc(kEventCapacity * sizeof(struct epoll_event));
    if (!m_events) return false;
    if ((m_queue = epoll_create(8)) <= 0) {
        free(m_events); m_events = nullptr;
        return false;
    } return true;
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_KQUEUE
    m_events = (struct kevent*)malloc(kEventCapacity * sizeof(struct kevent));
    if (!m_events) return false;
    if ((m_queue = kqueue()) <= 0) {
        free(m_events); m_events = nullptr;
        return false;
    } return true;
#else
    #error Not Implemented!
#endif
}
// -----------------------------------------------------------
void NetPoller::fini() {
#if SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_SELECT
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_EPOLL
    if (m_queue) { close(m_queue); m_queue = 0; }
    if (m_events) { free(m_events); m_events = nullptr; }
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_KQUEUE
    if (m_queue) { close(m_queue); m_queue = 0; }
    if (m_events) { free(m_events); m_events = nullptr; }
    if (m_changes) { free(m_changes); m_changes = nullptr; }
    m_changes_count = m_changes_capacity = 0;
#else
    #error Not Implemented!
#endif
}
// -----------------------------------------------------------
bool NetPoller::add(const RAW_SOCKET& fd, int event, void* ud) {
#if SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_SELECT
    if (fd >= FD_SETSIZE) return false;
    m_list.push_back(SelectItem{fd, event, ud});
    return true;
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_EPOLL
    struct epoll_event e;
    e.events = 0;
    if (event & kRead) e.events |= EPOLLIN;
    if (event & kWrite) e.events |= EPOLLOUT;
    e.data.fd = fd;
    e.data.ptr = ud;
    return epoll_ctl(m_queue, EPOLL_CTL_ADD, fd, &e) == 0;
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_KQUEUE
    if (!ensure_changes_size(m_changes_count +2)) return false;
    if (event & kRead) {
        EV_SET(&m_changes[m_changes_count++], fd, EVFILT_READ, EV_ADD|EV_ENABLE, 0, 0, ud);
    }
    if (event & kWrite) {
        EV_SET(&m_changes[m_changes_count++], fd, EVFILT_WRITE, EV_ADD|EV_ENABLE, 0, 0, ud);
    }
    return true;
#else
    #error Not Implemented!
#endif
}
// -----------------------------------------------------------
bool NetPoller::remove(const RAW_SOCKET& fd) {
#if SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_SELECT
    for (auto it=m_list.begin(); it != m_list.end(); ++it) {
        if (it->fd == fd) {
            m_list.erase(it);
            return true;
        }
    } return false;
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_EPOLL
    struct epoll_event e;
    e.data.fd = fd;
    e.events = 0;
    e.data.ptr = nullptr;
    return epoll_ctl(m_queue, EPOLL_CTL_DEL, fd, &e ) == 0;
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_KQUEUE
    if (!ensure_changes_size(m_changes_count +2)) return false;
    EV_SET(&m_changes[m_changes_count++], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
    EV_SET(&m_changes[m_changes_count++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
    return true;
#else
    #error Not Implemented!
#endif
}
// -----------------------------------------------------------
bool NetPoller::modify(const RAW_SOCKET& fd, int event, void* ud) {
#if SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_SELECT
    for (auto it=m_list.begin(); it != m_list.end(); ++it) {
        if (it->fd == fd) {
            it->event = event;
            return true;
        }
    } return false;
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_EPOLL
    struct epoll_event e;
    e.events = 0;
    if (event & kRead) e.events |= EPOLLIN;
    if (event & kWrite) e.events |= EPOLLOUT;
    e.data.fd = fd;
    e.data.ptr = ud;
    return epoll_ctl(m_queue, EPOLL_CTL_MOD, fd, &e) == 0;
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_KQUEUE
    if (!ensure_changes_size(m_changes_count +2)) return false;
    EV_SET(&m_changes[m_changes_count++], fd, EVFILT_READ, event & kRead ? EV_ADD|EV_ENABLE : EV_DELETE, 0, 0, ud);
    EV_SET(&m_changes[m_changes_count++], fd, EVFILT_WRITE, event & kWrite ? EV_ADD|EV_ENABLE : EV_DELETE, 0, 0, ud);
    return true;
#else
    #error Not Implemented!
#endif
}
// -----------------------------------------------------------
bool NetPoller::poll(unsigned int milliseconds, std::function<bool(const RAW_SOCKET& fd, int event, void* ud)> cb) {
#if SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_SELECT
    fd_set rfds, wfds, efds;
    RAW_SOCKET maxfd = 0;
    struct timeval tv;
    tv.tv_sec = static_cast<decltype(tv.tv_sec)>(milliseconds) / 1000;
    tv.tv_usec = (static_cast<decltype(tv.tv_usec)>(milliseconds) % 1000) * 1000;
    FD_ZERO(&rfds); FD_ZERO(&wfds); FD_ZERO(&efds);
    for (auto& item : m_list) {
        FD_SET(item.fd, &efds);
        if (item.event & kRead) FD_SET(item.fd, &rfds);
        if (item.event & kWrite) FD_SET(item.fd, &wfds);
        if (maxfd < item.fd ) maxfd = item.fd;
    }
    int events = select(static_cast<int>(maxfd+1), &rfds, &wfds, &efds, &tv);
    if (events <= 0) return false;
    for (auto it=m_list.begin(); it != m_list.end(); ) {
        int event = 0;
        if (FD_ISSET(it->fd, &efds)) event |= kClose;
        if (FD_ISSET(it->fd, &rfds)) event |= kRead;
        if (FD_ISSET(it->fd, &wfds)) event |= kWrite;
        if (cb(it->fd, event, it->ud)) {
            it++;
        } else {
            it = m_list.erase(it);
        }
    }
    return true;
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_EPOLL
    int events = epoll_wait(m_queue, m_events, kEventCapacity, milliseconds);
    if (events <= 0 ) return false;
    for (int i=0; i < events; i++) {
        int ev = 0;
        if (m_events[i].events & EPOLLIN) ev |= kRead;
        if (m_events[i].events & EPOLLOUT) ev |= kWrite;
        RAW_SOCKET fd = m_events[i].data.fd;
        if (!cb(fd, ev, m_events[i].data.ptr)) {
            remove(fd);
        }
    }
    return true;
#elif SIGNAL_POLL_IMPLEMENTATION == SIGNAL_POLL_KQUEUE
    struct timespec tv;
    tv.tv_sec = static_cast<decltype(tv.tv_sec)>(milliseconds) / 1000;
    tv.tv_nsec = (static_cast<decltype(tv.tv_nsec)>(milliseconds) % 1000) * 1000000;
    int events = kevent(m_queue, m_changes, m_changes_count, m_events, kEventCapacity, &tv);
    m_changes_count = 0;
    if ( events <= 0 ) return false;
    for (int i=0; i < events; i++) {
        int ev = 0;
        if (m_events[i].filter & EVFILT_READ) ev |= kRead;
        if (m_events[i].filter & EVFILT_WRITE) ev |= kWrite;
        RAW_SOCKET fd = static_cast<RAW_SOCKET>(m_events[i].ident);
        void* ud = static_cast<void*>(m_events[i].udata);
        if (ud && !cb(fd, ev, ud)) {
            remove(fd);
        }
    }
    return true;
#else
    #error Not Implemented!
#endif
}
// -----------------------------------------------------------
