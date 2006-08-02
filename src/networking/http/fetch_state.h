#ifndef _NETWORKING_HTTP_FETCH_STATE_H_
#define _NETWORKING_HTTP_FETCH_STATE_H_

#include <ace/Message_Block.h>

#include <string>

#include "request.h"
#include "handler.h"

namespace networking {
namespace http {

/**
 * Internal type used by fetcher to keep track of the received and
 * sent data.
 */
struct fetch_state {
    ACE_Message_Block buf_send;
    ACE_Message_Block buf_recv;

    ACE_HANDLE handle;
    handler   *fetch_handler;
};

} // ns http
} // ns networking

#endif //_NETWORKING_HTTP_REQUEST_H_
