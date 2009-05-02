#ifndef _NETWORKING_HTTP_FETCHER_H_
#define _NETWORKING_HTTP_FETCHER_H_

#include <ace/Connector.h>
#include <ace/SOCK_Connector.h>

#include "handler.h"
#include "request.h"
#include "fetcher_handler.h"

namespace networking {
namespace http {

    /**
     * Simple class for fetching HTTP data
     */
    class fetcher {
        ACE_Connector<fetcher_handler, ACE_SOCK_CONNECTOR> *_connector;
        ACE_Time_Value _timeout;
    public:
        enum status {
            FETCH_OK,
            FETCH_CONNECT_ERROR
        };

        fetcher();
        virtual ~fetcher();

        inline void timeout(const ACE_Time_Value &t) { _timeout = t; }

        status fetch(const request &req, handler *h);
        status fetch(const url     &u,   handler *h);
    };

} // ns http
} // ns networking

#endif //_HTTP_FETCHER_H_
