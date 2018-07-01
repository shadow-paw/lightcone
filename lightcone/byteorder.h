#ifndef LIGHTCONE_BYTEORDER_H__
#define LIGHTCONE_BYTEORDER_H__

#include <stdint.h>
// -----------------------------------------------------------
#if defined(PLATFORM_WIN32) || defined(PLATFORM_WIN64)
#include <stdlib.h>
  #define LIGHTCONE_ENDIANLITTLE

#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID) || defined(PLATFORM_BSD) || defined(PLATFORM_SOLARIS)
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define LIGHTCONE_ENDIANLITTLE
  #else
    #define LIGHTCONE_ENDIANBIG
  #endif

#elif defined(PLATFORM_OSX)
  #if defined(__i386__) || defined(__x86_64__)
    #define LIGHTCONE_ENDIANLITTLE
  #elif defined(__ppc__)
    #define LIGHTCONE_ENDIANBIG
  #endif

#elif defined(PLATFORM_IOS)
  #define LIGHTCONE_ENDIANLITTLE

#else
  #error Not Implemented!
#endif

namespace lightcone {
// -----------------------------------------------------------
class ByteOrder {
 public:
#ifdef LIGHTCONE_ENDIANLITTLE
    // host to endian little
    static uint16_t htoel(uint16_t v) { return v; }
    static uint32_t htoel(uint32_t v) { return v; }
    static uint64_t htoel(uint64_t v) { return v; }
    // endian little to host
    static uint16_t eltoh(uint16_t v) { return v; }
    static uint32_t eltoh(uint32_t v) { return v; }
    static uint64_t eltoh(uint64_t v) { return v; }
  #if defined(WIN32) || defined(WIN64)
    // host to network order
    static uint16_t hton(uint16_t v) { return _byteswap_ushort(v); }
    static uint32_t hton(uint32_t v) { return _byteswap_ulong(v); }
    static uint64_t hton(uint64_t v) { return _byteswap_uint64(v); }
    // network order to host
    static uint16_t ntoh(uint16_t v) { return _byteswap_ushort(v); }
    static uint32_t ntoh(uint32_t v) { return _byteswap_ulong(v); }
    static uint64_t ntoh(uint64_t v) { return _byteswap_uint64(v); }
  #else
    // host to network order
    static uint16_t hton(uint16_t v) { return __builtin_bswap16(v); }
    static uint32_t hton(uint32_t v) { return __builtin_bswap32(v); }
    static uint64_t hton(uint64_t v) { return __builtin_bswap64(v); }
    // network order to host
    static uint16_t ntoh(uint16_t v) { return __builtin_bswap16(v); }
    static uint32_t ntoh(uint32_t v) { return __builtin_bswap32(v); }
    static uint64_t ntoh(uint64_t v) { return __builtin_bswap64(v); }
  #endif
#else
    // host to endian little
    static uint16_t htoel(uint16_t v) { return __builtin_bswap16(v); }
    static uint32_t htoel(uint32_t v) { return __builtin_bswap32(v); }
    static uint64_t htoel(uint64_t v) { return __builtin_bswap64(v); }
    // endian little to host
    static uint16_t eltoh(uint16_t v) { return __builtin_bswap16(v); }
    static uint32_t eltoh(uint32_t v) { return __builtin_bswap32(v); }
    static uint64_t eltoh(uint64_t v) { return __builtin_bswap64(v); }
    // host to network order
    static uint16_t hton(uint16_t v) { return v; }
    static uint32_t hton(uint32_t v) { return v; }
    static uint64_t hton(uint64_t v) { return v; }
    // network order to host
    static uint16_t ntoh(uint16_t v) { return v; }
    static uint32_t ntoh(uint32_t v) { return v; }
    static uint64_t ntoh(uint64_t v) { return v; }
#endif
};
// -----------------------------------------------------------
}  // namespace lightcone

#undef LIGHTCONE_ENDIANLITTLE
#undef LIGHTCONE_ENDIANBIG

#endif  // LIGHTCONE_BYTEORDER_H__
