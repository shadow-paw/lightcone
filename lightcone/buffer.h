#ifndef LIGHTCONE_BUFFER_H__
#define LIGHTCONE_BUFFER_H__

#include <stdint.h>
#include <stddef.h>

namespace lightcone {
// -----------------------------------------------------------
//! Network buffer
class Buffer {
 public:
    //! default constructor
    Buffer();
    ~Buffer();
    //! copy constructor
    //! \param[in] o Object to copy
    //! \exception std::bad_alloc()
    Buffer(const Buffer& o);
    //! move constructor
    //! \param[in] o Object to move
    Buffer(Buffer&& o);
    //! copy assignment
    //! \param[in] o Object to copy
    //! \exception std::bad_alloc()
    Buffer& operator=(const Buffer& o);
    //! move assignment
    //! \param[in] o Object to move
    Buffer& operator=(Buffer&& o);

    // implicit cast to uint8_t
    operator uint8_t*() { return m_buffer + m_head; }
    operator const uint8_t*() const { return m_buffer + m_head; }

    //! Get size of buffer
    //! \return size of buffer, in bytes
    size_t size() const { return m_size; }
    //! Release any allocated memory, reset to initial state
    void free();
    //! Ensure a certain size of memory after end of buffer.
    //! \param[out] tail Receive the pointer to tail of buffer, which size bytes are writable. Maybe nullptr.
    //! \param[in] size number of bytes to reserve
    //! \return true on success, false on fail with no side-effect.
    //! \sa reserve_end
    bool reserve_begin(uint8_t** tail, size_t size);
    //! Commit bytes to buffer after reserve_begin. On success, size() is increased by commit_size.
    //! \param[in] commit_size number of bytes to commit
    //! \return true on success, false on fail with no side-effect.
    bool reserve_end(size_t commit_size);
    //! Trim/Skip bytes from begining of buffer.
    //! \param[in] size number of bytes to trim
    //! \return true on success, false on fail with no side-effect.
    bool trim_head(size_t size);
    //! Trim bytes from end of buffer. On success, size() is reduced by size.
    //! \param[in] size number of bytes to trim
    //! \return true on success, false on fail with no side-effect.
    bool trim_tail(size_t size);

 private:
    bool realloc(size_t size);

 private:
    uint8_t* m_buffer;
    size_t   m_allocated;
    size_t   m_head, m_size;
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_BUFFER_H__
