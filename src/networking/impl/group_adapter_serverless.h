#ifndef _NETWORKING_GROUP_ADAPTER_SERVERLESS_H_
#define _NETWORKING_GROUP_ADAPTER_SERVERLESS_H_

#include <netcomgrp/serverless/group.h>
#include <netcomgrp/serverless/event_observer.h>

#include "../http/fetcher.h"
#include "../group_handler.h"

#include "group_adapter_base.h"
#include "group_handler_base.h"
#include "dht_operation.h"

namespace networking {
class group_adapter_serverless
            : public group_adapter_base,
            public group_handler_notify,
            public dht::event_observer,
            public netcomgrp::serverless::event_observer,
            public dht_target,
            public http::handler
{
    dht_operation                *_dht_oper;
    netcomgrp::serverless::group *_sgrp;
    netcomgrp::addr_inet_type     _external_ip;
    http::fetcher                 _external_ip_fetcher;
    bool                          _external_ip_fetching;
    std::string                   _grp_id;
    chat_gaming::user             _self;

    // contains a count of the number of times connect has been tried for
    // int _dht_user_connecting;
    int _connect_try;
    // bool _dht_clients_updating;

    int  _connect();
    int  _cond_group_join();
    void _external_ip_fetch();
    void _external_ip_set(const std::string &ip);
protected:
    virtual netcomgrp::group  *create_group();
    virtual const std::string &join_group();

    virtual group_handler_base *create_handler(netcomgrp::group *g);
public:
    group_adapter_serverless(ACE_Reactor *r = NULL);
    virtual ~group_adapter_serverless();

    virtual int  connect(const chat_gaming::user &self);
    virtual int  disconnect();
    virtual void update(const chat_gaming::user &u);

    virtual void update_server_nodes();

    // dht::event_observer interface
    virtual int state_changed(int dht_state);
    // netcomgrp::serverless::event_observer interface
    virtual int extra_state_changed(int);

    // dht_target interface
    virtual void dht_ready_to_connect();
    virtual void dht_ready_to_disconnect();
    virtual void dht_update_server_nodes_done(int err);
    // group_handler_notify interface
    virtual void group_state_changed(int state);
    // http::handler interface
    virtual int handle_response(const http::response &resp);
    virtual int handle_error(int reason, const char *details);
    virtual int handle_close();
};

} // ns networking

#endif //_NETWORKING_GROUP_ADAPTER_SERVERLESS_H_
