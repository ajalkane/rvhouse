#ifndef _APP_OPTIONS_H_
#define _APP_OPTIONS_H_

#include "common.h"

#define NETCOMGRP_SERVERLESS_DEFAULT_PORT 2301
#define DEFAULT_LANGUAGE "English"
#define ROUTER_FW_HELP_URL \
	"http://revolt.speedweek.net/rv_house/index.php?section=router_fw_help"

class app_options {
private:
    bool _debug;
public:
    app_options();

    // Initializes options from app configuration file
    void init();

    inline bool debug() const { return _debug; }

    inline size_t limit_chat_msg() const { return 1024; }
};

#endif //_APP_OPTIONS_H_
