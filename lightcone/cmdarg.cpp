#include <string.h>
#include <string>
#include "cmdarg.h"

namespace lightcone {
// -----------------------------------------------------------
bool CmdArg::parse(int argc, const char* const* argv) {
    for (int i=1; i < argc; i++) {
        const char* p = strchr(argv[i], '=');
        if (p) {
            _args.emplace(std::string(argv[i], size_t(p - argv[i])), std::string(p+1));
        } else {
            _args.emplace(std::string(argv[i]), "");
        }
    } return true;
}
bool CmdArg::contains(const std::string& key) const {
    return _args.find(key) != _args.end();
}
std::string CmdArg::first(const std::string& key, const std::string& defvalue) const {
    auto it = _args.find(key);
    if (it == _args.end()) return defvalue;
    return it->second;
}
bool CmdArg::all(const std::string& key, std::function<bool(const std::string& value)> cb) const {
    auto pair = _args.equal_range(key);
    for (auto it=pair.first; it != pair.second; ++it) {
        if (!cb(it->second)) return false;
    } return true;
}
// -----------------------------------------------------------
}  // namespace lightcone
