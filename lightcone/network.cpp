#include <signal.h>
#include "network.h"

namespace lightcone {
// -----------------------------------------------------------
void Network::start() {
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    WSADATA wsad;
    WSAStartup(MAKEWORD(1, 1), &wsad);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_OSX) || defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
    signal(SIGPIPE, SIG_IGN);
#else
    #error Not Implemented!
#endif
}
// -----------------------------------------------------------
void Network::stop() {
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    WSACleanup();
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_OSX) || defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
    signal(SIGPIPE, SIG_DFL);
#else
    #error Not Implemented!
#endif
}
// -----------------------------------------------------------
}  // namespace lightcone
