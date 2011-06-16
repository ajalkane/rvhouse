#ifndef REGISTER_WINDOW_H_
#define REGISTER_WINDOW_H_

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

#include "../../common.h"
#include "../../messaging/message.h"
#include "../../messaging/message_string.h"
#include "../message_handler.h"
#include "../watched_object.h"

namespace gui {
namespace window {

class register_user 
    : public QDialog,
      public message_handler,
      public watched_object
{
    Q_OBJECT
    
    QLineEdit *_user_field;
    QLineEdit *_pass_field;
    QLineEdit *_pas2_field;
    QLineEdit *_mail_field;
    
    QPushButton *_reg_button;
    QPushButton *_cancel_button;
    
    void _create_actions();
    void _create_widgets();
    void _create_layout();
    void _connect_signals();

    ::message *_register_message();
    
public:
    register_user(QWidget *parent);
    
    // long on_network_command(FXObject *from, FXSelector sel, void *);
    void handle_message(::message *msg);

public slots:
    void register_clicked();
};

} // ns window
} // ns gui

#endif //REGISTER_WINDOW_H_
