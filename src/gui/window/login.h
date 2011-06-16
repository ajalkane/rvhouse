#ifndef LOGIN_WINDOW_H_
#define LOGIN_WINDOW_H_

#include <string>

#include <QDialog>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QPushButton>
#include <QWidget>
#include <QString>

#include "../../common.h"
#include "../../messaging/message.h"
#include "../message_handler.h"
#include "../watched_object.h"

namespace gui {
namespace window {

class login 
  : public QDialog,
    public message_handler,
    public watched_object
{
    Q_OBJECT

    QLineEdit *_user_field;
    QLineEdit *_pass_field;

    QPushButton   *_log_button;
    QPushButton   *_reg_button;
    QPushButton   *_quit_button;
    
    std::string _user;
    std::string _pass;
    bool        _user_validated;
    
    void _create_actions();
    void _create_widgets();
    void _create_layout();
    void _connect_signals();

    ::message *_login_message();

public:
    login(QWidget *owner);
    virtual ~login();
    
    const std::string &user() const;
    const std::string &pass() const;
    const std::string &user(const std::string &u);
    const std::string &pass(const std::string &p);
    bool               user_validated() const;
    bool               auto_connect() const;
    
    void handle_message      (::message *msg);

public slots:
    void login_clicked();
    void register_clicked();
};

} // ns window
} // ns gui

#endif //LOGIN_WINDOW_H__
