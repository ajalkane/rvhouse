#ifndef _NETWORKING_HTTP_REQUEST_H_
#define _NETWORKING_HTTP_REQUEST_H_

#include <string>

#include "url.h"

namespace networking {
namespace http {
    
    /**
     * Encapsulates HTTP request type
     */
    class request {
        std::string _method;
        url         _url;
        
    public:
        inline request();
        inline request(const url &u); // std::string &server, const std::string &file);
        inline request(const request &o);
        
        virtual ~request();
        
        inline const url         &target_url() const;
        inline const std::string &method    () const;
        
        inline request &operator=(const request &o);
    };

    inline request::request() : _method("GET") {
    }

    inline request::request(const url &u) : _method("GET"), _url(u) {
    }

    inline request::request(const request &o) {
        *this = o;
    }
    inline const url &request::target_url() const {
        return _url;
    }
    
    inline const std::string &request::method() const {
        return _method;
    }

    inline request &request::operator=(const request &o) {
        _url    = o._url;
        _method = o._method;
        return *this;
    }
} // ns http
} // ns networking

#endif //_NETWORKING_HTTP_REQUEST_H_
