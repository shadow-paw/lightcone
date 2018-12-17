#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <new>
#include <stdexcept>

namespace lightcone {
// -----------------------------------------------------------
Buffer::Buffer() {
    m_buffer = nullptr;
    m_allocated = m_head = m_size = 0;
}
Buffer::~Buffer() {
    free();
}
Buffer::Buffer(const Buffer& o) {
    m_buffer = nullptr;
    m_allocated = m_head = m_size = 0;
    if (!realloc(o.m_size)) throw std::bad_alloc();
    m_head = 0;
    m_size = o.m_size;
    memcpy(m_buffer, o.m_buffer + o.m_head, o.m_size);
}
Buffer::Buffer(Buffer&& o) {
    m_buffer = o.m_buffer; o.m_buffer = nullptr;
    m_allocated = o.m_allocated; o.m_allocated = 0;
    m_head = o.m_head; o.m_head = 0;
    m_size = o.m_size; o.m_size = 0;
}
Buffer& Buffer::operator=(const Buffer& o) {
    free();
    if (!realloc(o.m_size)) throw std::bad_alloc();
    m_head = 0;
    m_size = o.m_size;
    memcpy(m_buffer, o.m_buffer + o.m_head, o.m_size);
    return *this;
}
Buffer& Buffer::operator=(Buffer&& o) {
    free();
    m_buffer = o.m_buffer; o.m_buffer = nullptr;
    m_allocated = o.m_allocated; o.m_allocated = 0;
    m_head = o.m_head; o.m_head = 0;
    m_size = o.m_size; o.m_size = 0;
    return *this;
}
bool Buffer::realloc(size_t size) {
    // pad size to 16 bytes
    size = (((size + 15) >> 4) << 4);
    if (m_allocated < size) {
        uint8_t* p = static_cast<uint8_t*>(::realloc(m_buffer, size));
        if (!p) return false;
        m_buffer = p;
        m_allocated = size;
    }
    return true;
}
void Buffer::free() {
    if (m_buffer) {
        ::free(m_buffer);
        m_buffer = nullptr;
    }
    m_allocated = m_head = m_size = 0;
}
bool Buffer::reserve_begin(uint8_t** tail, size_t size) {
    if (!realloc(m_size + size)) return false;
    if (m_allocated <= m_head + m_size + size) {
        memmove(m_buffer, m_buffer+m_head, m_size);
        m_head = 0;
    }
    if (tail) *tail = m_buffer + m_head + m_size;
    return true;
}
bool Buffer::reserve_end(size_t commit_size) {
    if (commit_size > 0) {
        if (m_head + m_size + commit_size > m_allocated) return false;
        m_size += commit_size;
    } return true;
}
bool Buffer::trim_head(size_t size) {
    if (m_size < size) return false;
    m_head += size;
    m_size -= size;
    if (m_size == 0) m_head = 0;  // reset head when buffer empty
    return true;
}
bool Buffer::trim_tail(size_t size) {
    if (m_size < size) return false;
    m_size -= size;
    if (m_size == 0) m_head = 0;  // reset head when buffer empty
    return true;
}
// -----------------------------------------------------------
}  // namespace lightcone
