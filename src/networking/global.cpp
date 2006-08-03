#include <ace/ACE.h>
#include <fx.h>

#include "../config_file.h"
#include "reporter/client.h"
#include "global.h"

namespace networking {
    
accessor<config_file *>      net_conf(NULL);
accessor<reporter::client *> net_report(NULL);

}
