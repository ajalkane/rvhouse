#include "main.h"
#include "app_options.h"
#include "config_file.h"

app_options::app_options()
  : _debug(false),
    _flash_main_chat(false),
    _flash_room_chat(false),
    _flash_new_room(false),
    _flash_new_user(false),
    _flash_new_user_in_room(false),
    _flash_nick(false)
{
}

void
app_options::init() {
    _debug = conf()->get<bool>("main/debug", false);
    _flash_main_chat = pref()->get<bool>("general/flash_main_chat", false);
    _flash_room_chat = pref()->get<bool>("general/flash_room_chat", false);
    _flash_new_room  = pref()->get<bool>("general/flash_new_room",  false);
    _flash_new_user  = pref()->get<bool>("general/flash_new_user",  false);
    _flash_new_user_in_room  = pref()->get<bool>("general/flash_new_user_in_room",  false);
    _flash_nick      = pref()->get<bool>("general/flash_nick",      false);
}
