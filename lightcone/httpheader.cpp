#include <string.h>
#include <string_view>
#include <algorithm>
#include <cstdlib>
#include "stringutil.h"
#include "httpheader.h"

namespace lightcone {
// -----------------------------------------------------------
HttpRequestHeader::HttpRequestHeader() {
    version = HTTP_1_1;
}
HttpRequestHeader::HttpRequestHeader(const std::string& to_action, const std::string& to_uri) {
    version = HTTP_1_1;
    action = to_action;
    uri = to_uri;
}
HttpRequestHeader::HttpRequestHeader(HttpRequestHeader&& o) :
                                     std::unordered_multimap<std::string, std::string>(std::move(o)) {
    version = o.version; o.version = HTTP_1_1;
    action = std::move(o.action);
    uri = std::move(o.uri);
}
HttpRequestHeader& HttpRequestHeader::operator=(HttpRequestHeader&& o) {
    std::unordered_multimap<std::string, std::string>::operator=(std::move(o));
    version = o.version; o.version = HTTP_1_1;
    action = std::move(o.action);
    uri = std::move(o.uri);
    return *this;
}
ssize_t HttpRequestHeader::read_from(const void* text, size_t len) {
    if (!text) return -1;
    if (len < 9) return 0;  // min http size
    size_t pos = 0, next = 0;
    auto in_text = std::string_view((const char*)text, len);
    auto header_len = in_text.find("\r\n\r\n");
    if (header_len == std::string::npos) return 0;
    header_len += 4;
    auto header = std::string_view(in_text.data(), header_len);
    // first line
    pos = header.find("\r\n");
    if (pos == std::string::npos) return -1;  // invalid header
    auto first_line = StringUtil::split(header.data(), pos, ' ');
    if (first_line.size() != 3) return -1;  // invalid header
    action = std::move(first_line[0]);
    uri = std::move(first_line[1]);
    if (first_line[2].compare("HTTP/1.0") == 0) {
        version = HTTP_1_0;
    } else if (first_line[2].compare("HTTP/1.1") == 0) {
        version = HTTP_1_1;
    } else {
        return -1;  // unsupported version
    }
    // read dict
    for (;;) {
        next = header.find_first_of("\r\n", pos);
        if (next == std::string_view::npos) break;
        if (next > pos) {
            auto line = header.substr(pos, next - pos - 1);
            if (auto colon = line.find_first_of(':'); colon != std::string_view::npos) {
                auto key = StringUtil::trim(std::string(line.data(), colon));
                auto value = StringUtil::trim(std::string(line.data() + colon + 1, line.length() - colon));
                // convert key to lower case for convenience
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                this->emplace(key, value);
            }
        }
        pos = next + 2;
    }
    return (ssize_t)header_len;
}
ssize_t HttpRequestHeader::write_to(void* buf, size_t len) const {
    if (!buf || len == 0) return -1;
    char *b = static_cast<char*>(buf);
    size_t wlen = (size_t)snprintf(b, len,
                                   "%s %s HTTP/%d.%d\r\n",
                                   action.c_str(), uri.c_str(),
                                   version >> 16, version & 0xffff);
    for (auto it=begin(); it != end(); ++it) {
        if (wlen >= len) return -1;
        wlen += (size_t)snprintf(b + wlen, len - wlen,
                                 "%s: %s\r\n",
                                 it->first.c_str(), it->second.c_str());
    }
    if (wlen+2 >= len) return -1;
    snprintf(b + wlen, len - wlen, "\r\n");
    return (ssize_t)(wlen + 2);
}
// -----------------------------------------------------------
HttpResponseHeader::HttpResponseHeader() {
    version = HTTP_1_1;
    status = 0;
}
HttpResponseHeader::HttpResponseHeader(int to_status) {
    version = HTTP_1_1;
    status = to_status;
}
HttpResponseHeader::HttpResponseHeader(HttpResponseHeader&& o) :
                                     std::unordered_multimap<std::string, std::string>(std::move(o)) {
    version = o.version; o.version = HTTP_1_1;
    status = o.status; o.status = 0;
}
HttpResponseHeader& HttpResponseHeader::operator=(HttpResponseHeader&& o) {
    std::unordered_multimap<std::string, std::string>::operator=(std::move(o));
    version = o.version; o.version = HTTP_1_1;
    status = o.status; o.status = 0;
    return *this;
}
ssize_t HttpResponseHeader::read_from(const void* text, size_t len) {
    if (!text) return -1;
    if (len < 9) return 0;  // min http size
    size_t pos = 0, next = 0;
    auto in_text = std::string_view((const char*)text, len);
    auto header_len = in_text.find("\r\n\r\n");
    if (header_len == std::string::npos) return 0;
    header_len += 4;
    auto header = std::string_view(in_text.data(), header_len);
    // first line
    pos = header.find("\r\n");
    if (pos == std::string::npos) return -1;  // invalid header
    auto first_line = std::string_view(header.data(), pos);
    if (strncmp(first_line.data(), "HTTP/1.0 ", 8) == 0) {
        version = HTTP_1_0;
    } else if (strncmp(first_line.data(), "HTTP/1.1 ", 8) == 0) {
        version = HTTP_1_1;
    } else {
        return -1;  // unsupported version
    }
    if ((next = first_line.find_first_not_of(' ', 8)) == std::string_view::npos) {
        return -1;  // lack of status
    }
    status = std::atoi(first_line.data() + next);
    // read dict
    for (;;) {
        next = header.find_first_of("\r\n", pos);
        if (next == std::string_view::npos) break;
        if (next > pos) {
            auto line = header.substr(pos, next - pos - 1);
            if (auto colon = line.find_first_of(':'); colon != std::string_view::npos) {
                auto key = StringUtil::trim(std::string(line.data(), colon));
                auto value = StringUtil::trim(std::string(line.data() + colon + 1, line.length() - colon));
                // convert key to lower case for convenience
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                this->emplace(key, value);
            }
        }
        pos = next + 2;
    }
    return (ssize_t)header_len;
}
ssize_t HttpResponseHeader::write_to(void* buf, size_t len) const {
    if (!buf || len == 0) return -1;
    char *b = static_cast<char*>(buf);
    size_t wlen = (size_t)snprintf(b, len,
                                   "HTTP/%d.%d %d %s\r\n",
                                   version >> 16, version & 0xffff,
                                   status, status_message(status));
    for (auto it=begin(); it != end(); ++it) {
        if (wlen >= len) return -1;
        wlen += (size_t)snprintf(b + wlen, len - wlen,
                                 "%s: %s\r\n",
                                 it->first.c_str(), it->second.c_str());
    }
    if (wlen+2 >= len) return -1;
    snprintf(b + wlen, len - wlen, "\r\n");
    return (ssize_t)(wlen + 2);
}
const char* HttpResponseHeader::status_message(int status) {
    if (auto it = _status_messages.find(status); it != _status_messages.end()) {
        return it->second;
    } else if (auto it2 = _status_messages.find(0); it2 != _status_messages.end()) {
        return it2->second;
    }
    return nullptr;
}
const std::unordered_map<int, const char*> HttpResponseHeader::_status_messages = {
    {200, "OK"                            },
    {404, "Not Found"                     },
    {100, "Continue"                      },
    {101, "Web Socket Protocol Handshake" },
    {201, "Created"                       },
    {206, "Partial Content"               },
    {250, "Low on Storage Space"          },
    {300, "Multiple Choices"              },
    {301, "Moved Permanently"             },
    {302, "Moved Temporarily"             },
    {303, "See Other"                     },
    {304, "Not Modified"                  },
    {305, "Use Proxy"                     },
    {400, "Bad Request"                   },
    {401, "Unauthorized"                  },
    {402, "Payment Required"              },
    {403, "Forbidden"                     },
    {405, "Method Not Allowed"            },
    {406, "Not Acceptable"                },
    {407, "Proxy Authentication Required" },
    {408, "Request Time-out"              },
    {410, "Gone"                          },
    {411, "Length Required"               },
    {412, "Precondition Failed"           },
    {413, "Request Entity Too Large"      },
    {414, "Request-URI Too Large"         },
    {415, "Unsupported Media Type"        },
    {451, "Parameter Not Understood"      },
    {452, "Conference Not Found"          },
    {453, "Not Enough Bandwidth"          },
    {454, "Session Not Found"             },
    {455, "Method Not Valid in This State"},
    {456, "Header Field Not Valid for Resource"},
    {457, "Invalid Range"                 },
    {458, "Parameter Is Read-Only"        },
    {459, "Aggregate operation not allowed"},
    {460, "Only aggregate operation allowed"},
    {461, "Unsupported transport"         },
    {462, "Destination unreachable"       },
    {480, "Temporarity Not Available"     },
    {500, "Internal Server Error"         },
    {501, "Not Implemented"               },
    {502, "Bad Gateway"                   },
    {503, "Service Unavailable"           },
    {504, "Gateway Time-out"              },
    {505, "RTSP Version not supported"    },
    {551, "Option not supported"          },
    {0,   "Undefined"                     }
};
// -----------------------------------------------------------
}  // namespace lightcone
