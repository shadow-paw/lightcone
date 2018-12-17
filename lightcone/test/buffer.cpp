#include <stdio.h>
#include <utility>
#include "lightcone/lightcone.h"
#include "unittest.h"

// -----------------------------------------------------------
bool buffer_access() {
    lightcone::Buffer buf;
    uint8_t *p = nullptr;
    const int size = 4096;
    // init buffer with sequential bytes
    if (!buf.reserve_begin(&p, size)) return false;
    for (int i=0; i < size; i++) {
        p[i] = static_cast<uint8_t>(i & 0xff);
    }
    buf.reserve_end(size);
    // verify buffer
    if (buf.size() != size) return false;
    for (int i=0; i < size; i++) {
        if (buf[i] != static_cast<uint8_t>(i & 0xff)) return false;
    }
    // trim head
    buf.trim_head(3);
    if (buf.size() != size -3) return false;
    for (int i=0; i < size-3; i++) {
        if (buf[i] != static_cast<uint8_t>((i+3) & 0xff)) return false;
    }
    // trim tail
    buf.trim_tail(3);
    if (buf.size() != size -6) return false;
    for (int i=0; i < size-6; i++) {
        if (buf[i] != static_cast<uint8_t>((i+3) & 0xff)) return false;
    }
    // free
    buf.free();
    if (buf.size() != 0) return false;
    return true;
}
// -----------------------------------------------------------
bool buffer_copy() {
    lightcone::Buffer buf1, buf2;
    uint8_t *p = nullptr;
    const int size = 4096;
    if (!buf1.reserve_begin(&p, size)) return false;
    for (int i=0; i < size; i++) {
        p[i] = static_cast<uint8_t>(i & 0xff);
    }
    buf1.reserve_end(size);
    buf2 = buf1;
    if (buf2.size() != size) return false;
    for (int i=0; i < size; i++) {
        if (buf2[i] != static_cast<uint8_t>(i & 0xff)) return false;
    }
    return true;
}
// -----------------------------------------------------------
bool buffer_move() {
    lightcone::Buffer buf1, buf2;
    uint8_t *p = nullptr;
    const int size = 4096;
    if (!buf1.reserve_begin(&p, size)) return false;
    for (int i=0; i < size; i++) {
        p[i] = static_cast<uint8_t>(i & 0xff);
    }
    buf1.reserve_end(size);
    buf2 = std::move(buf1);
    if (buf1.size() != 0) return false;
    if (buf2.size() != size) return false;
    for (int i=0; i < size; i++) {
        if (buf2[i] != static_cast<uint8_t>(i & 0xff)) return false;
    }
    return true;
}
// -----------------------------------------------------------
int main(int argc, char* argv[]) {
    UnitTest t(__FILE__);
    t.run("buffer_access", buffer_access);
    t.run("buffer_copy", buffer_copy);
    t.run("buffer_move", buffer_move);
    return t.failed ? 1 : 0;
}
// -----------------------------------------------------------
