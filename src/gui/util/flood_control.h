#ifndef _GUI_UTIL_FLOOD_CONTROL_H_
#define _GUI_UTIL_FLOOD_CONTROL_H_

#include "ace/Time_Value.h"

#include "../../common.h"

namespace gui {
namespace util {

// Used by classes wishing to have flood control for input
class flood_control {
protected:
    ACE_Time_Value _flood_last_sent_message;
    int            _flood_control;
public:
    flood_control();

    bool allow_send();
};

} // ns util
} // ns gui

#endif /*FLOOD_CONTROL_H_*/
