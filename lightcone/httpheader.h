#ifndef LIGHTCONE_HTTPHEADER_H__
#define LIGHTCONE_HTTPHEADER_H__

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <unordered_map>

namespace lightcone {
// -----------------------------------------------------------
class HttpRequestHeader : public std::unordered_multimap<std::string, std::string> {
 public:
    enum Version {
        HTTP_1_0 = 0x00010000,
        HTTP_1_1 = 0x00010001,
    };
    Version version;
    std::string action;
    std::string uri;

    HttpRequestHeader();
    explicit HttpRequestHeader(const std::string& action, const std::string& uri);
    //! move constructor
    //! \param[in] o Object to move
    HttpRequestHeader(HttpRequestHeader&& o);
    //! move assignment
    //! \param[in] o Object to move
    HttpRequestHeader& operator=(HttpRequestHeader&& o);

    // serialize
    ssize_t read_from(const void* text, size_t len);
    ssize_t write_to(void* buf, size_t len) const;
};
class HttpResponseHeader : public std::unordered_multimap<std::string, std::string> {
 public:
    enum Version {
        HTTP_1_0 = 0x00010000,
        HTTP_1_1 = 0x00010001,
    };
    Version version;
    int status;

    HttpResponseHeader();
    explicit HttpResponseHeader(int status);
    //! move constructor
    //! \param[in] o Object to move
    HttpResponseHeader(HttpResponseHeader&& o);
    //! move assignment
    //! \param[in] o Object to move
    HttpResponseHeader& operator=(HttpResponseHeader&& o);

    // serialize
    ssize_t read_from(const void* text, size_t len);
    ssize_t write_to(void* buf, size_t len) const;

    static const char* status_message(int status);

 private:
    static const std::unordered_map<int, const char*> _status_messages;
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_HTTPHEADER_H__
