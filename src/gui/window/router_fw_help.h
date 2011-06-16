#ifndef GUI_ROUTER_FW_HELP_H
#define GUI_ROUTER_FW_HELP_H

#include "../../common.h"
#include "../modal_dialog_guard.h"

namespace gui {
namespace window {
    
class router_fw_help : public modal_dialog_guard {
public:
    router_fw_help(QWidget *parent);
    virtual ~router_fw_help();
};

} // ns window
} // ns gui

#endif // GUI_ROUTER_FW_HELP_H
