#ifndef _GUI_WINDOW_ABOUT_H_
#define _GUI_WINDOW_ABOUT_H_

#include <fx.h>

#include "../../common.h"

namespace gui {
namespace window {

class about : public FXDialogBox {
    FXDECLARE(about)

protected:
    about() {}

public:
    about(FXWindow *owner);
    virtual void create();
};

} // ns window
} // ns gui

#endif //_GUI_WINDOW_ABOUT_H_
