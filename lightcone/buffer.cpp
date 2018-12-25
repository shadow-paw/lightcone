#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <new>
#include <stdexcept>

namespace lightcone {
// -----------------------------------------------------------
Buffer::Buffer() {
    _buffer = nullptr;
    _allocated = _head = _size = 0;
}
Buffer::~Buffer() {
    free();
}
Buffer::Buffer(const Buffer& o) {
    _buffer = nullptr;
    _allocated = _head = _size = 0;
    if (!realloc(o._size)) throw std::bad_alloc();
    _head = 0;
    _size = o._size;
    memcpy(_buffer, o._buffer + o._head, o._size);
}
Buffer::Buffer(Buffer&& o) {
    _buffer = o._buffer; o._buffer = nullptr;
    _allocated = o._allocated; o._allocated = 0;
    _head = o._head; o._head = 0;
    _size = o._size; o._size = 0;
}
Buffer& Buffer::operator=(const Buffer& o) {
    free();
    if (!realloc(o._size)) throw std::bad_alloc();
    _head = 0;
    _size = o._size;
    memcpy(_buffer, o._buffer + o._head, o._size);
    return *this;
}
Buffer& Buffer::operator=(Buffer&& o) {
    free();
    _buffer = o._buffer; o._buffer = nullptr;
    _allocated = o._allocated; o._allocated = 0;
    _head = o._head; o._head = 0;
    _size = o._size; o._size = 0;
    return *this;
}
bool Buffer::realloc(size_t size) {
    // pad size to 16 bytes
    size = (((size + 15) >> 4) << 4);
    if (_allocated < size) {
        uint8_t* p = static_cast<uint8_t*>(::realloc(_buffer, size));
        if (!p) return false;
        _buffer = p;
        _allocated = size;
    }
    return true;
}
void Buffer::free() {
    if (_buffer) {
        ::free(_buffer);
        _buffer = nullptr;
    }
    _allocated = _head = _size = 0;
}
bool Buffer::reserve_begin(uint8_t** tail, size_t size) {
    if (!realloc(_size + size)) return false;
    if (_allocated <= _head + _size + size) {
        memmove(_buffer, _buffer+_head, _size);
        _head = 0;
    }
    if (tail) *tail = _buffer + _head + _size;
    return true;
}
bool Buffer::reserve_end(size_t commit_size) {
    if (commit_size > 0) {
        if (_head + _size + commit_size > _allocated) return false;
        _size += commit_size;
    } return true;
}
bool Buffer::trim_head(size_t size) {
    if (_size < size) return false;
    _head += size;
    _size -= size;
    if (_size == 0) _head = 0;  // reset head when buffer empty
    return true;
}
bool Buffer::trim_tail(size_t size) {
    if (_size < size) return false;
    _size -= size;
    if (_size == 0) _head = 0;  // reset head when buffer empty
    return true;
}
// -----------------------------------------------------------
}  // namespace lightcone
