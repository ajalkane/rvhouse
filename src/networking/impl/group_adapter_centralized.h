#ifndef _NETWORKING_GROUP_ADAPTER_CENTRALIZED_H_
#define _NETWORKING_GROUP_ADAPTER_CENTRALIZED_H_

#include <netcomgrp/centralized/client/group.h>

#include "group_adapter_base.h"
#include "group_handler_base.h"

namespace networking {
class group_adapter_centralized
            : public group_adapter_base,
            public group_handler_notify
{
    netcomgrp::centralized::client::group *_cgrp;
    std::string                           _grp_id;

    // bool _dht_clients_updating;

protected:
    virtual netcomgrp::group  *create_group();
    virtual const std::string &join_group();

    virtual group_handler_base *create_handler(netcomgrp::group *g);
public:
    group_adapter_centralized(ACE_Reactor *r = NULL);
    virtual ~group_adapter_centralized();

    // group_handler_notify interface
    void group_error(int err, const char *desc);
};

} // ns networking

#endif //_NETWORKING_GROUP_ADAPTER_CENTRALIZED_H_
