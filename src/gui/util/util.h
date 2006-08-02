#ifndef _GUI_UTIL_H_
#define _GUI_UTIL_H_

#include <fx.h>

#include <string>
#include <vector>

#include "../../common.h"

namespace util {

inline FXString button_text(const char *text, const char *tooltip) {
    FXString s;
    if (text) s = text;
    s += '\t';
    if (tooltip) s += tooltip;
    return s;
}

std::string private_message_channel_with(const std::string &user_id);

} // ns util

#endif // _GUI_UTIL_H_
