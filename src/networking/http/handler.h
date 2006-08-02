#ifndef _NETWORKING_HTTP_HANDLER_H_
#define _NETWORKING_HTTP_HANDLER_H_

#include <string>

#include "response.h"

namespace networking {
namespace http {

/**
 * User derived handler for the http request
 */
class handler {
public:
    enum {
        err_invalid_reply = 1,
        err_out_of_memory,
        err_timeout,
        err_socket,
    };

    virtual ~handler();

    virtual int handle_response(const response &resp) = 0;
    virtual int handle_timeout()                              { return 0; }
    virtual int handle_error(int reason, const char *details) { return 0; }
    virtual int handle_close()                                { return 0; }
};

} // ns http
} // ns networking

#endif //_NETWORKING_HTTP_REQUEST_H_
