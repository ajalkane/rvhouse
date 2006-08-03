#ifndef _REGISTER_WINDOW_H_
#define _REGISTER_WINDOW_H_

#include <fx.h>

#include "../../common.h"
#include "../../messaging/messenger.h"
#include "../../messaging/message.h"
#include "../../messaging/message_string.h"
#include "../message_handler.h"
#include "../watched_window.h"

namespace gui {
namespace window {

class register_user 
  : public FXDialogBox, 
    public message_handler,
    public watched_window 
{
    FXDECLARE(register_user)
    
    FXTextField *_user_field;
    FXTextField *_pass_field;
    FXTextField *_pas2_field;
    FXTextField *_mail_field;
    
    FXButton    *_reg_button;
    
    FXHorizontalFrame *_toolbar;

    ::message *_register_message();
    
protected:
    register_user() {}

public:
    enum {
        ID_REGISTER = FXDialogBox::ID_LAST,
    };
    
    register_user(FXWindow *owner);
    virtual void create();
    
    long on_network_command(FXObject *from, FXSelector sel, void *);    
    void handle_message(::message *msg);
};

} // ns window
} // ns gui

#endif //_REGISTER_WINDOW_H_
