#include <utility>

#include <QtGui>

#include "../../messaging/message_register.h"
#include "../../messaging/messenger.h"
#include "../../validation.h"
#include "register_user.h"

namespace gui {
namespace window {

register_user::register_user(QWidget *owner)
    : QDialog(owner),
      watched_object(this)
{
    ACE_DEBUG((LM_DEBUG, "register_user::register_user\n"));
    this->setWindowTitle(langstr("register_win/title"));

    _create_actions();
    _create_widgets();
    _create_layout();
    _connect_signals();

}

void
register_user::_create_actions() {
    ACE_DEBUG((LM_DEBUG, "register_user::_create_actions\n"));
}

void
register_user::_create_widgets() {
    ACE_DEBUG((LM_DEBUG, "register_user::_create_widgets\n"));
    _user_field = new QLineEdit(this);
    _pass_field = new QLineEdit(this);
    _pas2_field = new QLineEdit(this);
    _mail_field = new QLineEdit(this);
    _pass_field->setEchoMode(QLineEdit::Password);
    _pas2_field->setEchoMode(QLineEdit::Password);
    
    _reg_button    = new QPushButton(langstr("register_win/register"), this);
    _cancel_button = new QPushButton(langstr("common/cancel_button"), this);

    _user_field->setMaxLength(16);
    _pass_field->setMaxLength(16);
    _pas2_field->setMaxLength(16);
    _mail_field->setMaxLength(256);
}

void
register_user::_connect_signals() {
    ACE_DEBUG((LM_DEBUG, "login::_connect_signals\n"));

    connect(_reg_button,    SIGNAL(clicked()), this, SLOT(register_clicked()));
    connect(_cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
}

void
register_user::_create_layout() {
    ACE_DEBUG((LM_DEBUG, "login::_create_layout\n"));
    QVBoxLayout      *l          = new QVBoxLayout;
    QFormLayout      *form       = new QFormLayout;
    QDialogButtonBox *button_box = new QDialogButtonBox;

    QFrame      *separator = new QFrame;
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    form->addRow(langstr("register_win/user_id"),    _user_field);
    form->addRow(langstr("register_win/password"),   _pass_field);
    form->addRow(langstr("register_win/repassword"), _pas2_field);
    form->addRow(langstr("register_win/email"),      _mail_field);

    _reg_button->setDefault(true);
    button_box->addButton(_reg_button, QDialogButtonBox::ActionRole);
    button_box->addButton(_cancel_button, QDialogButtonBox::RejectRole);

    l->addLayout(form);
    l->addWidget(separator, 0, Qt::AlignBottom);
    l->addWidget(button_box);

    this->setLayout(l);
}

void
register_user::register_clicked() {
    ::message *msg = _register_message();
    if (msg) {
        _reg_button->setDisabled(true);
        net_messenger()->send_msg(msg);
    }
}

::message *
register_user::_register_message() {
    const char *error = NULL;
    if (_user_field->text().isEmpty()) {
        error = langstr("register_win/fill_user");
    } else if (_pass_field->text().isEmpty()) {
        error = langstr("register_win/fill_pass");
    } else if (_pass_field->text() != _pas2_field->text()) {
        error = langstr("register_win/pass_mismatch");
    } else if (!validate_user_id(_user_field->text().toLatin1().constData())) {
        error = langstr("register_win/invalid_user");
    } else if (!validate_password(_pass_field->text().toLatin1().constData())) {
        error = langstr("register_win/invalid_pass");
    } else if (!validate_email(_mail_field->text().toLatin1().constData())) {
        error = langstr("register_win/invalid_email");
    }

    if (error != NULL) {
        QMessageBox msg_box(this);
        msg_box.setWindowTitle(langstr("register_win/reg_err_topic"));
        ACE_DEBUG((LM_DEBUG, "register_user::_register_message setting box text to '%s'\n", error));
        msg_box.setText(error);
        msg_box.setIcon(QMessageBox::Critical);
        msg_box.exec();
        return NULL;

    }
    return new message_register(::message::reg,
                                _user_field->text().toLatin1().constData(),
                                _pass_field->text().toLatin1().constData(),
                                _mail_field->text().toLatin1().constData()
                                );
}

void
register_user::handle_message(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "register_user: received msg id %d\n", msg->id()));
    const char *error = NULL;
    const char *success = NULL;

    switch (msg->id()) {
    case ::message::reg_err:
        error = langstr("register_win/server_down");
        break;
    case ::message::reg_fail:
        error = langstr("register_win/user_reserved");
        break;
    case ::message::reg_done:
        success = langstr("register_win/reg_ok");
        break;
    }

    if (error != NULL) {
        QMessageBox msg_box(this);
        msg_box.setWindowTitle(langstr("register_win/reg_err_topic"));
        msg_box.setText(error);
        msg_box.setIcon(QMessageBox::Critical);
        msg_box.exec();
        _reg_button->setEnabled(true);

    } else if (success != NULL) {
        QMessageBox msg_box(this);
        msg_box.setWindowTitle(langstr("register_win/reg_ok_topic"));
        msg_box.setText(success);
        msg_box.setIcon(QMessageBox::Information);
        msg_box.exec();

        _reg_button->setEnabled(true);
        emit accept();
    }
}

} // ns window
} // ns gui
