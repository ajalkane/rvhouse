#ifndef OS_UTIL_H_
#define OS_UTIL_H_

#include <string>

#include <QWidget>
#include "common.h"

namespace os {

int  alert(const char *topic, const char *text);
const std::string &app_dir();
void flash_window(QWidget *obj);
bool ensure_single_app_instance();
bool is_windows_64();
bool is_windows_vista_or_later();

} // ns os;

#endif //OS_UTIL_H_
