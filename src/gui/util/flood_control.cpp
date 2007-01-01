#include <ace/Time_Value.h>
#include <ace/OS.h>
#include <time.h>

#include "flood_control.h"

namespace gui {
namespace util {

flood_control::flood_control()
    : _flood_last_sent_message(0),
      _flood_control(0)
{}

bool
flood_control::allow_send(const FXString &t) {
    // Simple flood control
    ACE_Time_Value now(ACE_OS::gettimeofday());
    
    if (_flood_control < 0 || (now - _flood_last_sent_message).msec() >= 5000) 
        _flood_control = 0;
    else
        _flood_control += 2000 - (now - _flood_last_sent_message).msec();
    
    if (_flood_control >= 3000) {
        return false;
    }

    _flood_last_sent_message = now;
    
    return true;
}

} // ns util
} // ns gui

