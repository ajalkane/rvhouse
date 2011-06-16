#ifndef ROOM_WINDOW_H_
#define ROOM_WINDOW_H_

#include <string>

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
#include "../../rv_cmdline_builder.h"
#include "../view/chat.h"
#include "../view/users.h"
#include "../message_handler.h"
#include "../util/flood_control.h"
#include "size_restoring_window.h"

namespace gui {
namespace window {

class room :
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
    QAction *_action_settings;
    QAction *_action_launch;

    QWidget *_info_container;
    QLabel  *_info_picks;
    QLabel  *_info_laps;
    QLabel  *_info_players;

    QSplitter *_chat_users_splitter;

//    FXTextField       *_msg_field;
//    FXButton          *_edit_button;
//    FXButton          *_launch_button;
//    FXButton          *_share_button;
//    FXLabel           *_info_picks;
//    FXLabel           *_info_laps;
//    FXLabel           *_info_players;
    
    bool               _running_modal;
    bool               _hosting;
    bool               _host_sharing;
    
    // chat_gaming::room _room;
    chat_gaming::room::id_type _room_id;    
    chat_gaming::user::id_type _host_id;

    util::flood_control _flood_control;
        
    void _create_actions();
    void _create_toolbars();
    void _create_widgets();
    void _create_layout();
    void _connect_signals();

    void _handle_room_launch(::message *msg);
    void _handle_room_kick  (::message *msg);
    void _launcher_error(int err);
    void _launch_host();
    void _launch_join(chat_gaming::house::user_iterator host_ui);
    model::house::user_iterator _playing_host();
    void _launched_display();
    void _buttons_state();
    void _init();
    void _room_message(const chat_gaming::user &u, int grp);
    void _button_sharing_tracks_enable(bool enable_if_possible);
    void _set_room_cmdline(rv_cmdline_builder &builder);

public:
    room(const chat_gaming::room::id_type &id, QWidget *owner);
    virtual ~room();

    void update(int grp);
    
    void handle_message    (::message *msg);    

    // view::users::observer interface  
    virtual void user_added(const chat_gaming::user &u);
    virtual void user_removed(const chat_gaming::user &u);
    virtual void user_blocked(const std::string &display_id);
    
    virtual void sharing_tracks_changed(const chat_gaming::user &u,
                                        bool value);
    virtual void getting_tracks_changed(const chat_gaming::user &u,
                                        bool value);

public slots:
    void show();
    void edit_room();
    void launch_rv();
    void send_message();
};

} // ns window
} // ns gui

#endif //ROOM_WINDOW_H_
