#ifndef LIGHTCONE_TEST_UNITTEST_H__
#define LIGHTCONE_TEST_UNITTEST_H__

#include <stdio.h>
#include <string>
#include <functional>

class UnitTest {
 public:
    int tested, failed;

    UnitTest() = delete;
    explicit UnitTest(const std::string& title) : tested(0), failed(0) {
        printf("[T] %s\n", title.c_str());
    }
    bool run(const std::string& name, std::function<bool()> testfunc) {
        printf("  - %s...", name.c_str());
        fflush(stdout);
        bool r = testfunc();
        if (!r) failed++;
        tested++;
        printf("%s\n", r ? "OK" : "FAIL");
        return r;
    }
};

#endif  // LIGHTCONE_TEST_UNITTEST_H__
