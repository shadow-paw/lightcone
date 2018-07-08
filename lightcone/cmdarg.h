#ifndef LIGHTCONE_CMDARG_H__
#define LIGHTCONE_CMDARG_H__

#include <string>
#include <unordered_map>
#include <functional>

namespace lightcone {
// -----------------------------------------------------------
//! Command argument parser
class CmdArg {
 public:
    bool parse(int argc, const char* const* argv);
    bool contains(const std::string& key) const;
    std::string first(const std::string& key, const std::string& defvalue = "") const;
    bool all(const std::string& key, std::function<bool(const std::string& value)> cb) const;
 private:
    std::unordered_multimap<std::string, std::string> m_args;
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_CMDARG_H__
