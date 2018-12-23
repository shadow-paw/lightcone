#include <stdio.h>
#include <string.h>
#include <utility>
#include "lightcone/lightcone.h"
#include "unittest.h"

// -----------------------------------------------------------
int main(int argc, char* argv[]) {
    UnitTest t(__FILE__);
    t.run("request header", []() -> bool {
        char buf[4096] = {0};
        lightcone::HttpRequestHeader header("GET", "/foo/bar");
        header.emplace("content-type", "text/html");
        header.emplace("x-testing", "{ \"abcd\": \"def\" }");
        header.emplace("x-testing", "xyz 123");
        // write to buffer
        auto wlen = header.write_to(buf, sizeof(buf));
        if (wlen < 0) return false;
        // read it back
        lightcone::HttpRequestHeader r;
        if (r.read_from(buf, (size_t)wlen) < 0) return false;
        // check for difference
        if (header.version != r.version) return false;
        if (header.action.compare(r.action) != 0) return false;
        if (header.uri.compare(r.uri) != 0) return false;
        auto lhs = header.find("content-type")->second;
        auto rhs = r.find("content-type")->second;
        if (lhs.compare(rhs) != 0) return false;
        auto range = r.equal_range("x-testing");
        for (auto it=range.first; it != range.second; ++it) {
            if (it->second.compare("{ \"abcd\": \"def\" }") != 0 &&
                it->second.compare("xyz 123") != 0) return false;
        }
        return true;
    });
    t.run("response header", []() -> bool {
        char buf[4096] = {0};
        lightcone::HttpResponseHeader header(200);
        header.emplace("content-length", "1234");
        header.emplace("x-testing", "{ \"abcd\": \"def\" }");
        header.emplace("x-testing", "xyz 123");
        // write to buffer
        auto wlen = header.write_to(buf, sizeof(buf));
        if (wlen < 0) return false;
        // read it back
        lightcone::HttpResponseHeader r;
        if (r.read_from(buf, (size_t)wlen) < 0) return false;
        // check for difference
        if (header.version != r.version) return false;
        if (header.status != r.status) return false;
        auto lhs = header.find("content-length")->second;
        auto rhs = r.find("content-length")->second;
        if (lhs.compare(rhs) != 0) return false;
        auto range = r.equal_range("x-testing");
        for (auto it=range.first; it != range.second; ++it) {
            if (it->second.compare("{ \"abcd\": \"def\" }") != 0 &&
                it->second.compare("xyz 123") != 0) return false;
        }
        return true;
    });
    return t.failed ? 1 : 0;
}
// -----------------------------------------------------------
