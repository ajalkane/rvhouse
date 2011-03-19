#ifndef _APP_OPTIONS_H_
#define _APP_OPTIONS_H_

#include "common.h"

#define NETCOMGRP_SERVERLESS_DEFAULT_PORT 2301
#define DEFAULT_LANGUAGE "English"
#define ROUTER_FW_HELP_URL \
    "http://rvhouse.zackattackgames.com/index.php?section=router_fw_help"

#define ACCESSOR(Type, Name) \
private:\
    Type _ ## Name; \
public: \
    inline const Type & Name () const { return _ ## Name; } \
    inline const Type & Name (const Type &var) const { return _ ## Name; }

/**
 * In the app_options class there is only the options
 * that are checked that often that it is thought
 * they'd better be able to access quickly instead
 * of going through the config_file interface.
 */
class app_options {
private:
    bool _debug;
public:
    app_options();

    // Initializes options from app configuration file
    void init();

    inline bool debug() const { return _debug; }

    inline size_t limit_chat_msg() const { return 1024; }

    ACCESSOR(bool, flash_main_chat)
    ACCESSOR(bool, flash_room_chat)
    ACCESSOR(bool, flash_new_room)
    ACCESSOR(bool, flash_new_user)
    ACCESSOR(bool, flash_new_user_in_room)
    ACCESSOR(bool, flash_nick)
};

#endif //_APP_OPTIONS_H_
