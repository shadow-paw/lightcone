#ifndef LIGHTCONE_CALENDAR_H__
#define LIGHTCONE_CALENDAR_H__

#include <stdint.h>
#include <chrono>

namespace lightcone {
// -----------------------------------------------------------
class Calendar {
 public:
    static uint64_t now() {
        return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now().time_since_epoch()).count();
    }
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_CALENDAR_H__
