#ifndef LIGHTCONE_STRINGUTIL_H__
#define LIGHTCONE_STRINGUTIL_H__

#include <stddef.h>
#include <string_view>
#include <string>
#include <vector>

namespace lightcone {
// -----------------------------------------------------------
//! Wrapper for sockaddr
class StringUtil {
 public:
    //! Split string
    //! \param[in] s source string to get splitted, must be null-terminated.
    //! \param[in] delimiter delimiter for split.
    //! \return list of splitted string.
    static std::vector<std::string> split(const char* s, char delimiter);
    //! Split string
    //! \param[in] s source string to get splitted, not necessary null-terminated.
    //! \param[in] len length of s, in bytes.
    //! \param[in] delimiter delimiter for split.
    //! \return list of splitted string.
    static std::vector<std::string> split(const char* s, size_t len, char delimiter);
    //! Split string
    //! \param[in] s source string to get splitted.
    //! \param[in] delimiter delimiter for split.
    //! \return list of splitted string.
    static std::vector<std::string> split(const std::string_view& s, char delimiter);
    //! Split string
    //! \param[in] s source string to get splitted.
    //! \param[in] delimiter delimiter for split.
    //! \return list of splitted string.
    static std::vector<std::string> split(const std::string& s, char delimiter);
    //! Split string
    //! \param[in] s source string to get splitted, must be null-terminated.
    //! \param[in] delimiter delimiter for split.
    //! \return list of splitted string.
    static std::vector<std::string> split(const char* s, const char* delimiter);
    //! Split string
    //! \param[in] s source string to get splitted, not necessary null-terminated.
    //! \param[in] len length of s, in bytes.
    //! \param[in] delimiter delimiter for split.
    //! \return list of splitted string.
    static std::vector<std::string> split(const char* s, size_t len, const char* delimiter);
    //! Split string
    //! \param[in] s source string to get splitted.
    //! \param[in] delimiter delimiter for split.
    //! \return list of splitted string.
    static std::vector<std::string> split(const std::string_view& s, const char* delimiter);
    //! Split string
    //! \param[in] s source string to get splitted.
    //! \param[in] delimiter delimiter for split.
    //! \return list of splitted string.
    static std::vector<std::string> split(const std::string& s, const char* delimiter);
    //! Trim whitespaces from string (space and tab)
    //! \param[in] s source string to get splitted.
    //! \return trimmed string.
    static std::string trim(const std::string& s);
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_STRINGUTIL_H__
