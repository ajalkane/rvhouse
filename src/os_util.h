#ifndef _OS_UTIL_H_
#define _OS_UTIL_H_

#include <string>

#include <fx.h>
#include "common.h"

namespace os {

int  alert(const char *topic, const char *text);
const std::string &app_dir();
void flash_window(FXWindow *win);
bool ensure_single_app_instance();

} // ns os;

#endif //_OS_UTIL_H_
