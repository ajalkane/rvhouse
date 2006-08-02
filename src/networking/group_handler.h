#ifndef _NETWORKING_GROUP_HANDLER_H_
#define _NETWORKING_GROUP_HANDLER_H_

#include <netcomgrp/group.h>

#include "../messaging/messenger.h"

namespace networking {
// TODO away the whole file, not needed
class group_handler : public netcomgrp::event_observer {
public:
    group_handler(netcomgrp::group *grp);
    virtual ~group_handler();

};
} // ns networking

#endif //_NETWORKING_GROUP_HANDLER_H_
