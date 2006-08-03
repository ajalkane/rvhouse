#ifndef _NETWORKING_GLOBAL_H_
#define _NETWORKING_GLOBAL_H_

#include "../accessor.h"

class config_file;

namespace networking {

// Forward declaration
namespace reporter {
    class client;
}

extern accessor<class config_file *>    net_conf;
extern accessor<class reporter::client *> net_report;
    

}

#endif // _NETWORKING_GLOBAL_H_
