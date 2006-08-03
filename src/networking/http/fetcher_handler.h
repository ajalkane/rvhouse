#ifndef _NETWORKING_HTTP_FETCHER_HANDLER_H_
#define _NETWORKING_HTTP_FETCHER_HANDLER_H_

#include <ace/Svc_Handler.h>
#include <ace/SOCK_Stream.h>
#include <ace/Connector.h>
#include <ace/SOCK_Connector.h>

#include <string>
#include <map>

#include "handler.h"
#include "request.h"
#include "fetch_state.h"

namespace networking {
namespace http {
    
    /**
     * Internal classed, used by fetcher to handle one connection
     */
    class fetcher_handler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> {
        typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;
        
        ACE_Message_Block *_recv_mb;
        handler           *_handler;
        
        const static int _input_buffer_size = 4096;
        const static int _max_header_size   = 1024;
        const static int _max_response_size = 32768;
        
        int _handle_response();
        
        // Due to template instantiation etc., need a default constructor
        // accessible by ACE_Connector
        fetcher_handler();
        friend class ACE_Connector<fetcher_handler, ACE_SOCK_CONNECTOR>;
    public:
        fetcher_handler(const request &req, handler *h);
        virtual ~fetcher_handler();
        
        void fetch(const request &req, handler *h);
                   
        // ACE_Svc_Handler interface
        virtual int open(void *p);
        virtual int handle_input (ACE_HANDLE fd=ACE_INVALID_HANDLE);
        virtual int handle_output(ACE_HANDLE fd=ACE_INVALID_HANDLE);
        virtual int handle_close (ACE_HANDLE handle, ACE_Reactor_Mask close_mask);
        virtual int handle_timeout(const ACE_Time_Value &tv, const void *);
    };

} // ns http
} // ns networking

#endif //_HTTP_FETCHER_H_
