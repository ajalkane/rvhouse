#include <utility>

#include <QtGui>

#include "../../messaging/message_login.h"
#include "../../messaging/message_string.h"
#include "../../messaging/messenger.h"
#include "../../util.h"
#include "../../validation.h"
//#include "../util/util.h"
#include "login.h"
#include "register_user.h"

namespace gui {
namespace window {

login::login(QWidget *owner)
    : QDialog(owner),
      watched_object(this),
      _user_validated(false)
{
    ACE_DEBUG((LM_DEBUG, "login::login\n"));
    this->setWindowTitle(langstr("login_win/title"));
    
    _create_actions();
    _create_widgets();
    _create_layout();
    _connect_signals();

}

void
login::_create_actions() {
    ACE_DEBUG((LM_DEBUG, "login::_create_actions\n"));
}

void
login::_create_widgets() {
    ACE_DEBUG((LM_DEBUG, "login::_create_widgets\n"));
    _user_field = new QLineEdit(this);
    _pass_field = new QLineEdit(this);
    _pass_field->setEchoMode(QLineEdit::Password);
    
    _log_button  = new QPushButton(langstr("login_win/login"), this);
    _reg_button  = new QPushButton(langstr("login_win/register"), this);
    _quit_button = new QPushButton(langstr("login_win/quit"), this);
}

void
login::_connect_signals() {
    ACE_DEBUG((LM_DEBUG, "login::_connect_signals\n"));

    connect(_log_button,  SIGNAL(clicked()), this, SLOT(login_clicked()));
    connect(_reg_button,  SIGNAL(clicked()), this, SLOT(register_clicked()));
    connect(_quit_button, SIGNAL(clicked()), this, SLOT(reject()));
}

void
login::_create_layout() {
    ACE_DEBUG((LM_DEBUG, "login::_create_layout\n"));
    QVBoxLayout      *l          = new QVBoxLayout;
    QFormLayout      *form       = new QFormLayout;
    QDialogButtonBox *button_box = new QDialogButtonBox;

    QLabel      *label     = new QLabel(langstr("login_win/uses_rvzt_login"));
    QFrame      *separator = new QFrame;

    label->setDisabled(true);
    label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    separator->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    form->addRow(langstr("login_win/user_id"),  _user_field);
    form->addRow(langstr("login_win/password"), _pass_field);

    _log_button->setDefault(true);
    button_box->addButton(_log_button, QDialogButtonBox::ActionRole);
    button_box->addButton(_reg_button, QDialogButtonBox::ActionRole);
    button_box->addButton(_quit_button, QDialogButtonBox::RejectRole);

    l->addLayout(form);
    l->addWidget(label, 1, Qt::AlignHCenter|Qt::AlignBottom);
    l->addWidget(separator, 0, Qt::AlignBottom);
    l->addWidget(button_box);

    this->setLayout(l);
}

login::~login() {
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
    return _user;
}

const std::string &
login::pass(const std::string &p) {
    _pass = p;
    _pass_field->setText(_pass.c_str());
    return _pass;
}

bool
login::user_validated() const {
    return _user_validated;
}

bool
login::auto_connect() const {
    // Always auto connect for now. RV House does not have disconnect/connect buttons at the moment
    return true;
    // return _connect_check->getCheck();
}

void
login::login_clicked() {
    ::message *msg = _login_message();
    if (msg) {
        _user           = _user_field->text().toLatin1().constData();
        _pass           = _pass_field->text().toLatin1().constData();
        _user_validated = false;
        _log_button->setDisabled(true);
        _reg_button->setDisabled(true);

        net_messenger()->send_msg(msg);
    }
}

void
login::register_clicked() {
    register_user reg_win(this);
    reg_win.exec();
}

::message *
login::_login_message() {
    const char *error = NULL;
    if (_user_field->text().isEmpty()) {
        error = langstr("login_win/fill_user");
    }
    else if (_pass_field->text().isEmpty()) {
        error = langstr("login_win/fill_pass");
    }
    else if (!validate_user_id(_user_field->text().toLatin1().constData())) {
        error = langstr("login_win/invalid_user");
    }
    else if (!validate_password(_pass_field->text().toLatin1().constData())) {
        error = langstr("login_win/invalid_pass");
    }

    if (error != NULL) {
        QMessageBox msg_box(this);
        msg_box.setWindowTitle(langstr("login_win/login_err_topic"));
        ACE_DEBUG((LM_DEBUG, "login::_login_message setting box text to '%s'\n", error));
        msg_box.setText(error);
        msg_box.setIcon(QMessageBox::Critical);
        msg_box.exec();
        return NULL;
    }

    return new message_login(::message::login,
                             _user_field->text().toLatin1().constData(),
                             _pass_field->text().toLatin1().constData());
}

void
login::handle_message(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "login: received msg id %d\n", msg->id()));
    switch (msg->id()) {
    case ::message::login_err:
    {
        QMessageBox msg_box(this);
        msg_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msg_box.setWindowTitle(langstr("login_win/login_err_topic"));
        msg_box.setText(langstr("login_win/server_down"));
        msg_box.setIcon(QMessageBox::Warning);
        int ret = msg_box.exec();

        _log_button->setEnabled(true);
        _reg_button->setEnabled(true);
        if (ret == QMessageBox::Yes) {
            this->accept();
        }
    }
        break;
    case ::message::login_fail:
    {
        QMessageBox msg_box(this);
        msg_box.setWindowTitle(langstr("login_win/login_err_topic"));
        msg_box.setText(langstr("login_win/validation_err"));
        msg_box.setIcon(QMessageBox::Information);
        msg_box.exec();

        _log_button->setEnabled(true);
        _reg_button->setEnabled(true);
    }
        break;
    case ::message::login_done:
    {
        message_login *m = dynamic_ptr_cast<message_login>(msg);
        _log_button->setEnabled(true);
        _reg_button->setEnabled(true);
        _user           = m->user();
        _user_validated = true;
        this->accept();
    }
        break;
    }
}

} // ns window
} // ns gui
