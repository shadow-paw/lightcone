#ifndef LIGHTCONE_COPYABLE_H__
#define LIGHTCONE_COPYABLE_H__

namespace lightcone {
// -----------------------------------------------------------
//! \cond Skip this from doxygen
template <class T>
class NonCopyable {
 protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};
//! \endcond
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_COPYABLE_H__
