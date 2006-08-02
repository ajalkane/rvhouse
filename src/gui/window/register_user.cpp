#include <utility>

#include <fx.h>
#include <fxkeys.h>

#include "../../messaging/message_register.h"
#include "../../util.h"
#include "../../validation.h"
#include "register_user.h"

namespace gui {
namespace window {

FXDEFMAP(register_user) register_user_map[]= {
            FXMAPFUNC(SEL_COMMAND,  register_user::ID_REGISTER,
                      register_user::on_network_command),
        };

FXIMPLEMENT(register_user, FXDialogBox, register_user_map, ARRAYNUMBER(register_user_map));
// FXIMPLEMENT(register_user, FXMainWindow, NULL, 0);

register_user::register_user(FXWindow *owner)
        : FXDialogBox(owner, langstr("register_win/title"))
{
    FXMatrix *m = new FXMatrix(this, 2, MATRIX_BY_COLUMNS);

    new FXLabel(m, langstr("register_win/user_id"));
    _user_field = new FXTextField(m, 16);
    new FXLabel(m, langstr("register_win/password"));
    _pass_field = new FXTextField(m, 16, NULL, 0,
                                  TEXTFIELD_PASSWD|FRAME_SUNKEN|FRAME_THICK);
    new FXLabel(m, langstr("register_win/repassword"));
    _pas2_field = new FXTextField(m, 16, NULL, 0,
                                  TEXTFIELD_PASSWD|FRAME_SUNKEN|FRAME_THICK);
    new FXLabel(m, langstr("register_win/email"));
    _mail_field = new FXTextField(m, 16);

    new FXSeparator(this);
    FXHorizontalFrame *bframe = new FXHorizontalFrame(this, LAYOUT_CENTER_X);
    _reg_button = new FXButton(bframe, langstr("register_win/register"), NULL, this, ID_REGISTER);
    new FXButton(bframe, langstr("common/cancel_button"), NULL, this, ID_CANCEL);

    _user_field->setFocus();

    getAccelTable()->addAccel(MKUINT(KEY_F4,ALTMASK),this,FXSEL(SEL_COMMAND,ID_CANCEL));
}

void
register_user::create() {
    FXDialogBox::create();
    watched_window::create(this);
}

long
register_user::on_network_command(FXObject *from, FXSelector sel, void *) {
    ::message *msg = NULL;

    switch (FXSELID(sel)) {
    case ID_REGISTER:
        msg = _register_message();
        if (msg)
            _reg_button->disable();
        break;
    }

    if (msg) net_messenger()->send_msg(msg);

    return 1;
}

::message *
register_user::_register_message() {
    if (_user_field->getText().empty()) {
        FXMessageBox::error(this, FX::MBOX_OK,
                            langstr("register_win/reg_err_topic"),
                            langstr("register_win/fill_user"));
        return NULL;
    }
    if (_pass_field->getText().empty()) {
        FXMessageBox::error(this, FX::MBOX_OK,
                            langstr("register_win/reg_err_topic"),
                            langstr("register_win/fill_pass"));

        return NULL;
    }

    if (_pass_field->getText() != _pas2_field->getText()) {
        FXMessageBox::error(this, FX::MBOX_OK,
                            langstr("register_win/reg_err_topic"),
                            langstr("register_win/pass_mismatch"));

        return NULL;
    }

    if (!validate_user_id(_user_field->getText().text())) {
        FXMessageBox::error(this, FX::MBOX_OK,
                            langstr("register_win/reg_err_topic"),
                            langstr("register_win/invalid_user"));
        return NULL;
    }
    if (!validate_password(_pass_field->getText().text())) {
        FXMessageBox::error(this, FX::MBOX_OK,
                            langstr("register_win/reg_err_topic"),
                            langstr("register_win/invalid_pass"));
        return NULL;
    }
    if (!validate_email(_mail_field->getText().text())) {
        FXMessageBox::error(this, FX::MBOX_OK,
                            langstr("register_win/reg_err_topic"),
                            langstr("register_win/invalid_email"));
        return NULL;
    }

    return new message_register(::message::reg,
                                _user_field->getText().text(),
                                _pass_field->getText().text(),
                                _mail_field->getText().text()
                               );
}

void
register_user::handle_message(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "register_user: received msg id %d\n", msg->id()));

    switch (msg->id()) {
    case ::message::reg_err:
        FXMessageBox::error(this, FX::MBOX_OK,
                            langstr("register_win/reg_err_topic"),
                            langstr("register_win/server_down"));
        _reg_button->enable();
        break;
    case ::message::reg_fail:
        FXMessageBox::error(this, FX::MBOX_OK,
                            langstr("register_win/reg_err_topic"),
                            langstr("register_win/user_reserved"));
        _reg_button->enable();
        break;
    case ::message::reg_done:
        FXMessageBox::information(this, FX::MBOX_OK,
                                  langstr("register_win/reg_ok_topic"),
                                  langstr("register_win/reg_ok"));
        _reg_button->enable();
        handle(this, FXSEL(SEL_COMMAND, ID_CANCEL), NULL);
        break;
    }
}

} // ns window
} // ns gui
