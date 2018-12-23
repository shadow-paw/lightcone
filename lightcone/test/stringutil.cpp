#include <stdio.h>
#include <iostream>
#include <utility>
#include "lightcone/lightcone.h"
#include "unittest.h"

// -----------------------------------------------------------
int main(int argc, char* argv[]) {
    UnitTest t(__FILE__);
    t.run("split(const char * s, char delimiter)", []() -> bool {
        const char* text = "Hello     World ";
        auto splitted = lightcone::StringUtil::split(text, ' ');
        if (splitted.size() != 2) return false;
        if (splitted[0].compare("Hello") != 0) return false;
        if (splitted[1].compare("World") != 0) return false;
        return true;
    });
    t.run("split(const char* s, size_t len, char delimiter)", []() -> bool {
        const char text[] = "Hello World abc";
        auto splitted = lightcone::StringUtil::split(text, 11, ' ');
        if (splitted.size() != 2) return false;
        if (splitted[0].compare("Hello") != 0) return false;
        if (splitted[1].compare("World") != 0) return false;
        return true;
    });
    t.run("split(const std::string& s, char delimiter)", []() -> bool {
        const std::string text = "Hello World";
        auto splitted = lightcone::StringUtil::split(text, ' ');
        if (splitted.size() != 2) return false;
        if (splitted[0].compare("Hello") != 0) return false;
        if (splitted[1].compare("World") != 0) return false;
        return true;
    });
    t.run("split(const std::string_view& s, char delimiter)", []() -> bool {
        const char s[] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
        std::string_view text(s, sizeof(s));
        auto splitted = lightcone::StringUtil::split(text, ' ');
        if (splitted.size() != 2) return false;
        if (splitted[0].compare("Hello") != 0) return false;
        if (splitted[1].compare("World") != 0) return false;
        return true;
    });
    t.run("trim nothing", []() -> bool {
        const char text[] = "Testing 123";
        auto trimmed = lightcone::StringUtil::trim(text);
        return trimmed.compare("Testing 123") == 0;
    });
    t.run("trim all spaces", []() -> bool {
        const char text[] = "      ";
        auto trimmed = lightcone::StringUtil::trim(text);
        return trimmed.compare("") == 0;
    });
    t.run("trim L", []() -> bool {
        const char text[] = "    Testing 123";
        auto trimmed = lightcone::StringUtil::trim(text);
        return trimmed.compare("Testing 123") == 0;
    });
    t.run("trim R", []() -> bool {
        const char text[] = "Testing 123   ";
        auto trimmed = lightcone::StringUtil::trim(text);
        return trimmed.compare("Testing 123") == 0;
    });
    t.run("trim LR", []() -> bool {
        const char text[] = "    Testing 123  ";
        auto trimmed = lightcone::StringUtil::trim(text);
        return trimmed.compare("Testing 123") == 0;
    });
    return t.failed ? 1 : 0;
}
// -----------------------------------------------------------
