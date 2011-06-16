#ifndef GUI_WINDOW_VERSION_UPDATE_H
#define GUI_WINDOW_VERSION_UPDATE_H

#include "../../common.h"
#include "../../messaging/message_version.h"
#include "../modal_dialog_guard.h"

namespace gui {
namespace window {
    
class version_update : public modal_dialog_guard {
    message_version *_msg;
public:
    version_update(message_version *m, QWidget *parent);
    virtual ~version_update();
};

} // ns window
} // ns gui

#endif // GUI_WINDOW_VERSION_UPDATE
