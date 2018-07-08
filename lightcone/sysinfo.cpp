#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <stdlib.h>
#elif defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
  #include <sys/types.h>
  #include <sys/sysctl.h>
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID) || defined(PLATFORM_SOLARIS)
  #include <stdio.h>
  #include <stdlib.h>
#endif
#include <stddef.h>
#include "sysinfo.h"

namespace lightcone {
// -----------------------------------------------------------
int SysInfo::cpu_core() {
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
    // -------------------------------------------------------
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL, ptr;
    DWORD byteOffset, returnLen = 0;
    int num_core = 0;

    GetLogicalProcessorInformation(buffer, &returnLen);
    if (returnLen <= 0) return 1;
    if ((buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLen)) == NULL) return 1;
    if (!GetLogicalProcessorInformation(buffer, &returnLen)) {
        free(buffer);
        return 1;
    }
    byteOffset = 0;
    ptr = buffer;
    while (byteOffset < returnLen) {
        switch (ptr->Relationship) {
        case RelationProcessorCore:
            num_core++;
            break;
        }
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }
    free(buffer);
    return num_core > 0 ? num_core : 1;

#elif defined(PLATFORM_BSD) || defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    // -------------------------------------------------------
    int num_core = 1;
    size_t s = sizeof(num_core);
    sysctlbyname("hw.ncpu", &num_core, &s, NULL, 0);
    return num_core > 0 ? num_core : 1;

#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
    // -------------------------------------------------------
    char sz[64];
    FILE *f = popen("cat /proc/cpuinfo | grep processor | wc -l" , "r");
    if (f) {
        if (!fgets(sz, 64, f)) {
            pclose(f);
            return 1;
        }
        pclose(f);
        int num_core = atoi(sz);
        return num_core > 0 ? num_core : 1;
    } return 1;

#elif defined(PLATFORM_SOLARIS)
    // -------------------------------------------------------
    char sz[64];
    FILE *f = popen("kstat cpu_info | grep clock_MHz | wc -l" , "r");
    if (f) {
        if (!fgets(sz, 64, f)) {
            pclose(f);
            return 1;
        }
        pclose(f);
        int num_core = atoi(sz);
        return num_core > 0 ? num_core : 1;
    } return 1;
#else
  #error Not Implemented!
#endif
}
// -----------------------------------------------------------
}  // namespace lightcone
