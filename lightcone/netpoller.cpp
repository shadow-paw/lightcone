#include "threads.h"
#include "netpoller.h"

namespace lightcone {
// -----------------------------------------------------------
NetPoller::NetPoller() {
#if LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_SELECT
#elif LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_EPOLL
    _queue = 0;
    _events = nullptr;
#elif LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_KQUEUE
    _queue = 0;
    _events = nullptr;
    _changes = nullptr;
    _changes_count = _changes_capacity = 0;
#else
    #error Not Implemented!
#endif
}
NetPoller::~NetPoller() {
    fini();
}
#if LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_KQUEUE
bool NetPoller::ensure_changes_size(int size) {
    if ( size > _changes_capacity ) {
        struct kevent* p = (struct kevent*)realloc(_changes, ((size_t)size+16)*sizeof(struct kevent));
        if (!p) return false;
        _changes = p;
        _changes_capacity = size+16;
    } return true;
}
#endif
bool NetPoller::init() {
#if LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_SELECT
    return true;
#elif LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_EPOLL
    _events = (struct epoll_event*)malloc(kEventCapacity * sizeof(struct epoll_event));
    if (!_events) return false;
    if ((_queue = epoll_create(8)) <= 0) {
        free(_events); _events = nullptr;
        return false;
    } return true;
#elif LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_KQUEUE
    _events = (struct kevent*)malloc(kEventCapacity * sizeof(struct kevent));
    if (!_events) return false;
    if ((_queue = kqueue()) <= 0) {
        free(_events); _events = nullptr;
        return false;
    } return true;
#else
    #error Not Implemented!
#endif
}
void NetPoller::fini() {
#if LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_SELECT
#elif LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_EPOLL
    if (_queue) { close(_queue); _queue = 0; }
    if (_events) { free(_events); _events = nullptr; }
#elif LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_KQUEUE
    if (_queue) { close(_queue); _queue = 0; }
    if (_events) { free(_events); _events = nullptr; }
    if (_changes) { free(_changes); _changes = nullptr; }
    _changes_count = _changes_capacity = 0;
#else
    #error Not Implemented!
#endif
}
bool NetPoller::add(const RAW_SOCKET& fd, int event, void* ud) {
#if LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_SELECT
    if (fd >= FD_SETSIZE) return false;
    _list.push_back(SelectItem{fd, event, ud});
    return true;
#elif LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_EPOLL
    struct epoll_event e;
    e.events = 0;
    if (event & kRead) e.events |= EPOLLIN;
    if (event & kWrite) e.events |= EPOLLOUT;
    e.data.fd = fd;
    e.data.ptr = ud;
    return epoll_ctl(_queue, EPOLL_CTL_ADD, fd, &e) == 0;
#elif LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_KQUEUE
    if (!ensure_changes_size(_changes_count +2)) return false;
    if (event & kRead) {
        EV_SET(&_changes[_changes_count++], fd, EVFILT_READ, EV_ADD|EV_ENABLE, 0, 0, ud);
    }
    if (event & kWrite) {
        EV_SET(&_changes[_changes_count++], fd, EVFILT_WRITE, EV_ADD|EV_ENABLE, 0, 0, ud);
    }
    return true;
#else
    #error Not Implemented!
#endif
}
bool NetPoller::remove(const RAW_SOCKET& fd) {
#if LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_SELECT
    for (auto it=_list.begin(); it != _list.end(); ++it) {
        if (it->fd == fd) {
            _list.erase(it);
            return true;
        }
    } return false;
#elif LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_EPOLL
    struct epoll_event e;
    e.data.fd = fd;
    e.events = 0;
    e.data.ptr = nullptr;
    return epoll_ctl(_queue, EPOLL_CTL_DEL, fd, &e ) == 0;
#elif LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_KQUEUE
    if (!ensure_changes_size(_changes_count +2)) return false;
    EV_SET(&_changes[_changes_count++], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
    EV_SET(&_changes[_changes_count++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
    return true;
#else
    #error Not Implemented!
#endif
}
bool NetPoller::modify(const RAW_SOCKET& fd, int event, void* ud) {
#if LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_SELECT
    for (auto it=_list.begin(); it != _list.end(); ++it) {
        if (it->fd == fd) {
            it->event = event;
            return true;
        }
    } return false;
#elif LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_EPOLL
    struct epoll_event e;
    e.events = 0;
    if (event & kRead) e.events |= EPOLLIN;
    if (event & kWrite) e.events |= EPOLLOUT;
    e.data.fd = fd;
    e.data.ptr = ud;
    return epoll_ctl(_queue, EPOLL_CTL_MOD, fd, &e) == 0;
#elif LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_KQUEUE
    if (!ensure_changes_size(_changes_count +2)) return false;
    EV_SET(&_changes[_changes_count++], fd, EVFILT_READ, event & kRead ? EV_ADD|EV_ENABLE : EV_DELETE, 0, 0, ud);
    EV_SET(&_changes[_changes_count++], fd, EVFILT_WRITE, event & kWrite ? EV_ADD|EV_ENABLE : EV_DELETE, 0, 0, ud);
    return true;
#else
    #error Not Implemented!
#endif
}
bool NetPoller::poll(unsigned int milliseconds, std::function<bool(const RAW_SOCKET& fd, int event, void* ud)> cb) {
#if LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_SELECT
    fd_set rfds, wfds, efds;
    RAW_SOCKET maxfd = 0;
    struct timeval tv;
    tv.tv_sec = static_cast<decltype(tv.tv_sec)>(milliseconds) / 1000;
    tv.tv_usec = (static_cast<decltype(tv.tv_usec)>(milliseconds) % 1000) * 1000;
    FD_ZERO(&rfds); FD_ZERO(&wfds); FD_ZERO(&efds);
    for (auto& item : _list) {
        FD_SET(item.fd, &efds);
        if (item.event & kRead) FD_SET(item.fd, &rfds);
        if (item.event & kWrite) FD_SET(item.fd, &wfds);
        if (maxfd < item.fd ) maxfd = item.fd;
    }
    if (maxfd == 0) {
        Threads::usleep(1);
        return false;
    }
    int events = select(static_cast<int>(maxfd+1), &rfds, &wfds, &efds, &tv);
    if (events <= 0) return false;
    for (auto it=_list.begin(); it != _list.end(); ) {
        int event = 0;
        if (FD_ISSET(it->fd, &efds)) event |= kClose;
        if (FD_ISSET(it->fd, &rfds)) event |= kRead;
        if (FD_ISSET(it->fd, &wfds)) event |= kWrite;
        if (cb(it->fd, event, it->ud)) {
            it++;
        } else {
            it = _list.erase(it);
        }
    }
    return true;
#elif LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_EPOLL
    int events = epoll_wait(_queue, _events, kEventCapacity, milliseconds);
    if (events <= 0 ) return false;
    for (int i=0; i < events; i++) {
        int ev = 0;
        if (_events[i].events & EPOLLIN) ev |= kRead;
        if (_events[i].events & EPOLLOUT) ev |= kWrite;
        RAW_SOCKET fd = _events[i].data.fd;
        if (!cb(fd, ev, _events[i].data.ptr)) {
            remove(fd);
        }
    }
    return true;
#elif LIGHTCONE_POLL_IMPLEMENTATION == LIGHTCONE_POLL_KQUEUE
    struct timespec tv;
    tv.tv_sec = static_cast<decltype(tv.tv_sec)>(milliseconds) / 1000;
    tv.tv_nsec = (static_cast<decltype(tv.tv_nsec)>(milliseconds) % 1000) * 1000000;
    int events = kevent(_queue, _changes, _changes_count, _events, kEventCapacity, &tv);
    _changes_count = 0;
    if ( events <= 0 ) return false;
    for (int i=0; i < events; i++) {
        int ev = 0;
        if (_events[i].filter & EVFILT_READ) ev |= kRead;
        if (_events[i].filter & EVFILT_WRITE) ev |= kWrite;
        RAW_SOCKET fd = static_cast<RAW_SOCKET>(_events[i].ident);
        void* ud = static_cast<void*>(_events[i].udata);
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
}  // namespace lightcone
