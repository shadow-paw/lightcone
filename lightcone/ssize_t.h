#ifndef LIGHTCONE_SSIZE_T_H__
#define LIGHTCONE_SSIZE_T_H__

#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#elif defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    #include <sys/types.h>
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
    #include <sys/types.h>
#elif defined(PLATFORM_SOLARIS)
    #include <sys/types.h>
#else
    #error Not Implemented!
#endif

#endif  // LIGHTCONE_SSIZE_T_H__
