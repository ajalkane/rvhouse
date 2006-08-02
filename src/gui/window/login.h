#ifndef _LOGIN_WINDOW_H_
#define _LOGIN_WINDOW_H_

#include <string>

#include <fx.h>

#include "../../common.h"
#include "../../messaging/messenger.h"
#include "../../messaging/message.h"
#include "../../messaging/message_string.h"
#include "../message_handler.h"
#include "../watched_window.h"

namespace gui {
namespace window {

class login
            : public FXDialogBox,
            public message_handler,
            public watched_window
{
    FXDECLARE(login)

    FXTextField *_user_field;
    FXTextField *_pass_field;
    FXButton    *_log_button;
    FXButton    *_reg_button;
    FXCheckButton *_connect_check;

    FXHorizontalFrame *_toolbar;

    ::message *_login_message();

    std::string _user;
    std::string _pass;
    bool        _user_validated;

protected:
    login() {}

public:
    enum {
        ID_LOGIN = FXDialogBox::ID_LAST,
        ID_REGISTER,
    };

    login(FXWindow *owner);
    virtual ~login();
    virtual void create();

    const std::string &user() const;
    const std::string &pass() const;
    const std::string &user(const std::string &u);
    const std::string &pass(const std::string &p);
    bool               user_validated() const;
    bool               auto_connect() const;

    long on_network_command(FXObject *from, FXSelector sel, void *);
    void handle_message      (::message *msg);
};

} // ns window
} // ns gui

#endif //_LOGIN_WINDOW_H__
