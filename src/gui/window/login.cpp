#include <utility>

#include <fx.h>
#include <fxkeys.h>

#include "../../messaging/message_login.h"
#include "../../util.h"
#include "../../validation.h"
#include "../util/util.h"
#include "login.h"
#include "register_user.h"

namespace gui {
namespace window {

FXDEFMAP(login) login_map[]= {
    FXMAPFUNCS(SEL_COMMAND, login::ID_LOGIN, 
                            login::ID_REGISTER,
                            login::on_network_command),
};

FXIMPLEMENT(login, FXDialogBox, login_map, ARRAYNUMBER(login_map));
// FXIMPLEMENT(login, FXMainWindow, NULL, 0);

login::login(FXWindow *owner) 
    : FXDialogBox(owner, langstr("login_win/title")),
      _user_validated(false)
{
    FXMatrix *m = new FXMatrix(this, 2, MATRIX_BY_COLUMNS);
    
    new FXLabel(m, langstr("login_win/user_id")); // "User ID:");
    _user_field = new FXTextField(m, 16);
    new FXLabel(m, langstr("login_win/password"));
    _pass_field = new FXTextField(m, 16, NULL, 0,
                                  TEXTFIELD_PASSWD|FRAME_SUNKEN|FRAME_THICK);
    FXComposite *f = new FXHorizontalFrame(this, LAYOUT_CENTER_X);
    FXLabel *l = new FXLabel(f, langstr("login_win/uses_rvzt_login"), NULL, LABEL_NORMAL,
                             0,0,0,0, 0,0,0,0); // , NULL, JUSTIFY_CENTER_X|ICON_BEFORE_TEXT);
    l->disable();
    
    _connect_check = new FXCheckButton(this, "Auto connect");
    _connect_check->setCheck(true);
    // Hide for now, always connect automatically until disconnect/connect
    // functionality is implemented
    _connect_check->hide();
    
    new FXSeparator(this);
    FXHorizontalFrame *bframe = new FXHorizontalFrame(this, LAYOUT_CENTER_X);
    _log_button = new FXButton(
        bframe, langstr("login_win/login"), NULL, 
        this, ID_LOGIN, 
        BUTTON_NORMAL|BUTTON_INITIAL|BUTTON_DEFAULT /*,
        0,0,0,0,20,20*/
    );
    _reg_button = new FXButton(
        bframe, langstr("login_win/register"), NULL, 
        this, ID_REGISTER /*,
        BUTTON_NORMAL,
        0,0,0,0,20,20 */ 
    );
    new FXButton(
        bframe, langstr("login_win/quit"), NULL, 
        this, ID_CANCEL /*,
        BUTTON_NORMAL,
        0,0,0,0,20,20 */   
    );
    
    _log_button->setFocus();

    util::restore_size(this, "login_win");
    
    getAccelTable()->addAccel(MKUINT(KEY_F4,ALTMASK),this,FXSEL(SEL_COMMAND,ID_CANCEL));
}

void
login::create() {
    ACE_DEBUG((LM_DEBUG, "login::create2\n"));
    FXDialogBox::create();
    watched_window::create(this);   
}

login::~login() {
    ACE_DEBUG((LM_DEBUG, "login::dtor\n"));
    util::store_size(this, "login_win");
}

const std::string &
login::user() const {
    return _user;
}

const std::string &
login::pass() const {
    return _pass;
}

const std::string &
login::user(const std::string &u) {
    _user = u;
    _user_field->setText(_user.c_str());
    if (!_user.empty() && !_pass.empty())
        _log_button->setFocus();
    return _user;
}

const std::string &
login::pass(const std::string &p) {
    _pass = p;
    _pass_field->setText(_pass.c_str());
    if (!_user.empty() && !_pass.empty())
        _log_button->setFocus();
    return _pass;
}

bool
login::user_validated() const {
    return _user_validated;
}

bool
login::auto_connect() const {
    return _connect_check->getCheck();
}

long 
login::on_network_command(FXObject *from, FXSelector sel, void *) {
    ::message *msg = NULL;

    switch (FXSELID(sel)) {
    case ID_LOGIN:
        msg = _login_message();
        if (msg) {
            _user           = _user_field->getText().text();
            _pass           = _pass_field->getText().text();
            _user_validated = false;
            _log_button->disable();
            _reg_button->disable();
        }
        break;
    case ID_REGISTER:
    {
        ACE_DEBUG((LM_DEBUG, "Register window message received\n"));
        // Deleted automatically by house_app as is watched_window type     
        register_user *reg_win = new register_user(this);
        reg_win->show(PLACEMENT_SCREEN);
        if (!reg_win->execute(PLACEMENT_SCREEN)) {
            ACE_DEBUG((LM_DEBUG, "Reg cancel\n"));
        }
        delete reg_win;
    }
        break;
    }
    
    if (msg) net_messenger()->send_msg(msg);

    return 1;       
}

::message *
login::_login_message() {
    if (_user_field->getText().empty()) {
        FXMessageBox::error(this, FX::MBOX_OK, 
                            langstr("login_win/login_err_topic"),
                            langstr("login_win/fill_user"));
        return NULL;
    }
    if (_pass_field->getText().empty()) {
        FXMessageBox::error(this, FX::MBOX_OK, langstr("login_win/login_err_topic"),
                            langstr("login_win/fill_pass"));
        return NULL;
    }
    
    if (!validate_user_id(_user_field->getText().text())) {
        FXMessageBox::error(this, FX::MBOX_OK, langstr("login_win/login_err_topic"),
                            langstr("login_win/invalid_user"));
        return NULL;
    }
    if (!validate_password(_pass_field->getText().text())) {
        FXMessageBox::error(this, FX::MBOX_OK, langstr("login_win/login_err_topic"),
                            langstr("login_win/invalid_pass"));
        return NULL;
    }
            
    return new message_login(::message::login,
                             _user_field->getText().text(),
                             _pass_field->getText().text());
}

void
login::handle_message(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "login: received msg id %d\n", msg->id()));
    
    switch (msg->id()) {
    case ::message::login_err:
    {
        int ret = FXMessageBox::error(this, FX::MBOX_YES_NO, 
                  langstr("login_win/login_err_topic"),
                  langstr("login_win/server_down"));
        _log_button->enable();
        _reg_button->enable();
        if (ret == MBOX_CLICKED_YES) {
            handle(this, FXSEL(SEL_COMMAND, ID_ACCEPT), NULL);      
        }
    }
        break;
    case ::message::login_fail:
        FXMessageBox::error(this, FX::MBOX_OK, 
                            langstr("login_win/login_err_topic"),
                            langstr("login_win/validation_err"));
        _log_button->enable();
        _reg_button->enable();
        break;
    case ::message::login_done:
    {
        message_login *m = dynamic_ptr_cast<message_login>(msg);
        _log_button->enable();
        _reg_button->enable();
        _user           = m->user();
        _user_validated = true;
        handle(this, FXSEL(SEL_COMMAND, ID_ACCEPT), NULL);
        
    }
        break;
    }
}

} // ns window
} // ns gui
