#ifndef _NETWORKING_HTTP_URL_H_
#define _NETWORKING_HTTP_URL_H_

#include <ace/INET_Addr.h>
#include <string>

#include "../../common.h"
#include "../../exception.h"

namespace networking {
namespace http {
    
    /**
     * Encapsulates URL
     */
    class url {
        mutable ACE_INET_Addr _addr;
        mutable bool  _is_addr_set;
        std::string   _file;
        std::string   _host;
        std::string   _port;
        
        void _set_addr() const;
    public:
        inline url() : _is_addr_set(false) {}
        inline url(const std::string &url_string) 
        { parse(url_string); }

        virtual ~url();
                
        void parse(const std::string &url_string);
        
        inline const ACE_INET_Addr &addr() const;
        inline const std::string   &file() const;
        inline const std::string   &host() const;
        inline const std::string   &port() const;
        
        // inline url &operator=(const url &o);
    };

    /* inline url::url(const url &o) {
        *this = o;
    } */
    inline const ACE_INET_Addr &url::addr() const {
        if (!_is_addr_set) _set_addr();
        return _addr;
    }
    
    inline const std::string &url::file() const { return _file; }
    inline const std::string &url::host() const { return _host; }
    inline const std::string &url::port() const { return _port; }
    
    /*
    inline url &url::operator=(const url &o) {
        _addr = o._addr;
        _host = o._host;
        _port = o._port;
        _file = o._file;
        return *this;
    }*/
} // ns http
} // ns networking

#endif //_NETWORKING_HTTP_URL_H_
