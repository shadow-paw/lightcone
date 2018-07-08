#ifndef LIGHTCONE_SYSINFO_H__
#define LIGHTCONE_SYSINFO_H__

#include <stdint.h>
#include <stddef.h>

namespace lightcone {
// -----------------------------------------------------------
class SysInfo {
 public:
    //! Get number of cpu core
    //! \return number of cpu core.
    static int cpu_core();
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_SYSINFO_H__
