#ifndef _GUI_WATCHED_WINDOW_H_
#define _GUI_WATCHED_WINDOW_H_

#include <fx.h>

#include "../common.h"
#include "watched_object.h"

namespace gui {
/**
 * Reports back stuff to the main application of the window's state
 */
class watched_window : public watched_object{
    // FXWindow *_self;
public:
    watched_window();
    // watched_window(FXWindow *self);
    virtual ~watched_window();  
    void create(FXWindow *self);
};

} // ns gui

#endif //_GUI_WATCHED_WINDOW_H_
