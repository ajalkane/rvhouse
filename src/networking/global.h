#ifndef _NETWORKING_GLOBAL_H_
#define _NETWORKING_GLOBAL_H_

#include "../accessor.h"

class config_file;

namespace networking {

// Forward declarations
namespace reporter {
    class client;
}

namespace ip_block {
    class store;
}

extern accessor<class config_file *>      net_conf;
extern accessor<class reporter::client *> net_report;
extern accessor<class ip_block::store *>  net_ip_block;

}

#endif // _NETWORKING_GLOBAL_H_
