#include "group_handler_base.h"
#include "group_adapter_centralized.h"

#include "../../messaging/message_string.h"
#include "../worker.h"
#include "../global.h"

namespace networking {

group_adapter_centralized::group_adapter_centralized(
    ACE_Reactor *r
) : group_adapter_base(r), _cgrp(NULL)
{
    // TODO needs global config.h or from fxsettings
    _grp_id = net_conf()->get_value("net_centralized/server") + ':' +
              net_conf()->get_value("net_centralized/port");

    ACE_DEBUG((LM_DEBUG, "group_adapter_centralized: group id: %s\n",
              _grp_id.c_str()));              
}

group_adapter_centralized::~group_adapter_centralized() {
}

netcomgrp::group  *
group_adapter_centralized::create_group() {
    _cgrp = new netcomgrp::centralized::client::group;
    
    return _cgrp;
}

const std::string &
group_adapter_centralized::join_group() {
    return _grp_id;
}

group_handler_base *
group_adapter_centralized::create_handler(netcomgrp::group *g) {
    // return new group_handler_centralized(g, m);
    group_handler_base *h = new group_handler_base(g, message::ctz_group_base);
    h->notify(this);
    return h;
}

void
group_adapter_centralized::group_error(int err, const char *desc) {
    ACE_DEBUG((LM_DEBUG, "group_adapter_centralized::group_error: " \
              "received error (%d) %s\n", err, desc));
              
    switch (err) {
    case netcomgrp::group::err_timeout_connect:
    case netcomgrp::group::err_connect:
    {
        message_string *m = new message_string(
          message::ctz_group_server_unreachable,
          _grp_id
        );
        ACE_DEBUG((LM_DEBUG, "group_adapter_centralized: _grp_id %s, msg str: %s\n",
                  _grp_id.c_str(), m->str().c_str()));
                  
        gui_messenger()->send_msg(m);
    }
        break;
    }
}


} // ns networking
