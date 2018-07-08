#ifndef LIGHTCONE_NETINC_H__
#define LIGHTCONE_NETINC_H__

#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #undef WIN32_LEAN_AND_MEAN

#elif defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    #include <sys/types.h>
    #include <sys/ioctl.h>
    #include <sys/socket.h>
    #include <sys/sockio.h>
    #include <sys/time.h>
    #include <sys/event.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <net/if.h>
    #include <net/if_dl.h>

#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
    #include <sys/types.h>
    #include <sys/ioctl.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <net/if.h>
    #include <sys/epoll.h>

#elif defined(PLATFORM_SOLARIS)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/sockio.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <net/if.h>
    #include <net/route.h>
    #include <net/if_dl.h>

#else
    #error Not Implemented!
#endif

#endif  // LIGHTCONE_NETINC_H__
