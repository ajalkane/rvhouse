#ifndef PRIVATE_MESSAGE_WINDOW_H_
#define PRIVATE_MESSAGE_WINDOW_H_

#include <string>
#include <bitset>

#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QLabel>
#include <QString>
#include <QWidget>
#include <QVBoxLayout>
#include <QSplitter>

#include "../../chat_gaming/house.h"
#include "../../chat_gaming/room.h"
#include "../../common.h"
#include "../view/chat.h"
#include "../view/users.h"
#include "../message_handler.h"
#include "../util/slots.h"
#include "../util/flood_control.h"
#include "size_restoring_window.h"

namespace gui {
namespace window {

class private_message :
    public size_restoring_window<QMainWindow>,
    public message_handler,
    public view::users::observer
{
    Q_OBJECT

    typedef size_restoring_window<QMainWindow> super;

    view::chat  *_chat_view;
    view::users *_users_view;
    QLineEdit      *_msg_field;

    QAction *_action_quit;

    QSplitter *_chat_users_splitter;

    // chat_gaming::room _room;
    std::string _channel;
    std::string _user_id_str;

    // Used to hold a slot for a free private window position
    // IMPROVE Qt. Probably needs to be moved to rv_house class
    // static util::bit_slots<16> _slots;
    util::slot             _slot;
    util::flood_control    _flood_control;
    
    void _create_actions();
    void _create_toolbars();
    void _create_widgets();
    void _create_layout();
    void _connect_signals();

    void _init();


public:
    private_message(const std::string &user_id, QWidget *parent = NULL);
    // private_message(FXWindow *owner, const std::string &user_id);
    virtual ~private_message();
    
    inline const std::string &user_id_str() const { return _user_id_str; }
    inline const std::string &channel() const     { return _channel; }
    
    void handle_message    (::message *msg);    

    // view::users::observer interface  
    virtual void user_added(const chat_gaming::user &u);
    virtual void user_removed(const chat_gaming::user &u);

public slots:
    void send_message();
    void show();
};

} // ns window
} // ns gui

#endif //PRIVATE_MESSAGE_WINDOW_H_
