#ifndef _NETWORKING_HTTP_RESPONSE_H_
#define _NETWORKING_HTTP_RESPONSE_H_

#include <string>

namespace networking {
namespace http {
    
    /**
     * Encapsulates HTTP response type
     */
    class response {
        const char *_content;
        size_t      _content_len;
        
    public:
        response() : _content(NULL), _content_len(0) {}     
        ~response() {}
        
        inline const char *content    () const;
        inline size_t      content_len() const;

        inline void content    (const char *c);
        inline void content_len(size_t l);
    };

    inline const char *
    response::content() const     { return _content; }
    inline size_t
    response::content_len() const { return _content_len; }

    inline void 
    response::content(const char *c) { _content = c; }
    inline void 
    response::content_len(size_t l)  { _content_len = l; }

} // ns http
} // ns networking

#endif //_NETWORKING_HTTP_RESPONSE_H_
