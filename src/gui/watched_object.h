#ifndef _GUI_WATCHED_OBJECT_H_
#define _GUI_WATCHED_OBJECT_H_

#include <fx.h>

#include "../common.h"

namespace gui {
/**
 * Reports back stuff between main application <-> object of states
 */
class watched_object {
protected:
    FXObject *_self;
public:
    watched_object();
    // watched_window(FXWindow *self);
    virtual ~watched_object();  
    void create(FXObject *self);
};

} // ns gui

#endif //_GUI_WATCHED_OBJECT_H_
