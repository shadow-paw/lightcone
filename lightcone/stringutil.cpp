#include <string.h>
#include "stringutil.h"

namespace lightcone {
// -----------------------------------------------------------
std::vector<std::string> StringUtil::split(const std::string_view& s, char delimiter) {
    std::vector<std::string> result;
    size_t pos = 0, next = 0;
    for (;;) {
        next = s.find_first_of(delimiter, pos);
        if (next == std::string_view::npos) break;
        if (next > pos) {
            result.push_back(std::string(s.data() + pos, next - pos));
        }
        pos = next + 1;
    }
    if (pos < s.length()) {
        result.push_back(std::string(s.data() + pos, s.length() - pos));
    }
    return result;
}
std::vector<std::string> StringUtil::split(const std::string& s, char delimiter) {
    std::vector<std::string> result;
    size_t pos = 0, next = 0;
    for (;;) {
        next = s.find_first_of(delimiter, pos);
        if (next == std::string::npos) break;
        if (next > pos) {
            result.push_back(s.substr(pos, next - pos));
        }
        pos = next + 1;
    }
    if (pos < s.length()) {
        result.push_back(s.substr(pos));
    }
    return result;
}
std::vector<std::string> StringUtil::split(const char* s, size_t len, char delimiter) {
    if (!s) return {};
    return split(std::string_view(s, len), delimiter);
}
std::vector<std::string> StringUtil::split(const char* s, char delimiter) {
    if (!s) return {};
    return split(std::string_view(s), delimiter);
}
// -----------------------------------------------------------
std::vector<std::string> StringUtil::split(const std::string_view& s, const char* delimiter) {
    std::vector<std::string> result;
    size_t pos = 0, next = 0;
    for (;;) {
        next = s.find_first_of(delimiter, pos);
        if (next == std::string_view::npos) break;
        if (next > pos) {
            result.push_back(std::string(s.data() + pos, next - pos));
        }
        pos = next + 1;
    }
    if (pos < s.length()) {
        result.push_back(std::string(s.data() + pos, s.length() - pos));
    }
    return result;
}
std::vector<std::string> StringUtil::split(const std::string& s, const char* delimiter) {
    std::vector<std::string> result;
    size_t pos = 0, next = 0;
    for (;;) {
        next = s.find_first_of(delimiter, pos);
        if (next == std::string::npos) break;
        if (next > pos) {
            result.push_back(s.substr(pos, next - pos));
        }
        pos = next + 1;
    }
    if (pos < s.length()) {
        result.push_back(s.substr(pos));
    }
    return result;
}
std::vector<std::string> StringUtil::split(const char* s, size_t len, const char* delimiter) {
    if (!s) return {};
    return split(std::string_view(s, len), delimiter);
}
std::vector<std::string> StringUtil::split(const char* s, const char* delimiter) {
    if (!s) return {};
    return split(std::string_view(s), delimiter);
}
// -----------------------------------------------------------
std::string StringUtil::trim(const std::string& s) {
    const char WHITESPACES[] = " \t";
    auto startpos = s.find_first_not_of(WHITESPACES);
    if (startpos == std::string::npos) return {};
    auto endpos = s.find_last_not_of(WHITESPACES);
    if (endpos == std::string::npos) {
        return s.substr(startpos);
    }
    return s.substr(startpos, endpos - startpos + 1);
}
// -----------------------------------------------------------
}  // namespace lightcone
