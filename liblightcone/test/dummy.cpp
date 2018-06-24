#include "lightcone.h"
#include <stdio.h>

using lightcone::Foo;

// -----------------------------------------------------------
bool test_foo(){
    Foo foo;
    if (foo.bar(1, 2) != 3) return false;
    return true;
}
// -----------------------------------------------------------
bool run_tests() {
    if (!test_foo()) { printf ("FAILED. test_foo()\n"); return false; }
    return true;
}
// -----------------------------------------------------------
int main(int argc, char* argv[]) {
    bool success = run_tests();
    return success ? 0 : 1;
}
// -----------------------------------------------------------
