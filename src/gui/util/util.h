#ifndef _GUI_UTIL_H_
#define _GUI_UTIL_H_

#include <fx.h>

#include <string>
#include <vector>

#include "../../common.h"

namespace gui {
namespace util {

void store_size  (FXWindow *win, const char *section);
void restore_size(FXWindow *win, const char *section);

inline FXString button_text(const char *text, const char *tooltip) {
    FXString s;
    if (text) s = text;
    s += '\t';
    if (tooltip) s += tooltip;
    return s;
}

std::string private_message_channel_with(const std::string &user_id);

inline void create_default_button(
    FXComposite *p, const FXString &text,
    FXObject *t = NULL, FXSelector sel=0,
    FXint opts = 
        BUTTON_INITIAL|BUTTON_DEFAULT|
        FRAME_RAISED|FRAME_THICK,
    FXint pl = 20,
    FXint pr = 20
) {
    new FXButton(p, text, NULL, t, sel, opts, 0,0,0,0,pl,pr);
}

inline void create_button(
    FXComposite *p, const FXString &text,
    FXObject *t = NULL, FXSelector sel=0,
    FXint opts = BUTTON_NORMAL,
    FXint pl = 20,
    FXint pr = 20
) {
    new FXButton(p, text, NULL, t, sel, opts, 0,0,0,0,pl,pr);
}

inline FXComposite *framed_container(FXComposite *parent) {
    return new FXVerticalFrame(
            parent,
            LAYOUT_FILL_X|LAYOUT_FILL_Y|
            FRAME_SUNKEN,
            0,0,0,0, 0,
            0,0,0
    );
}
                
} // ns util
} // ns gui

#endif // _GUI_UTIL_H_
