#ifndef _NETWORKING_VERSION_UPDATE_CLIENT_H_
#define _NETWORKING_VERSION_UPDATE_CLIENT_H_

#include <ace/Reactor.h>

#include "../http/fetcher.h"

namespace networking {
namespace version_update {
    class client : public http::handler {
    public:
        client();
        virtual ~client();
        
        // If called, fetches the information about newest version
        // and after sending the information deletes self
        // Note, after calling this the caller must not use
        // the object anymore - it might be deleted already after
        // returning from this call.
        void fetch();
        
        // http::handler interface      
        virtual int handle_response(const http::response &resp);
        virtual int handle_error(int reason, const char *details);           
        virtual int handle_close();
    private:
        http::fetcher *_http_fetcher;
    };
} // ns version_update
} // ns networking

#endif //_NETWORKING_VERSION_UPDATE_CLIENT_H_
