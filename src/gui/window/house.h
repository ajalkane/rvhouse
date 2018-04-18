#ifndef HOUSE_WINDOW_H_
#define HOUSE_WINDOW_H_

#include <string>
#include <map>
#include <vector>

#include <time.h>
#include <stdlib.h>

#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QWidget>
#include <QString>
#include <QSplitter>

#include <ace/Time_Value.h>
#include <time.h>

#include "../../chat_gaming/user.h"
#include "../../messaging/message.h"
#include "../../messaging/message_block_users.h"
#include "../../multi_feed/user_item.h"
#include "../util/flood_control.h"
#include "../view/chat.h"
#include "../view/users.h"
#include "../view/rooms.h"
#include "../message_handler.h"

#include "../../networking/ip_block/store.h"

#include "size_restoring_window.h"

namespace gui {
namespace window {

class house :
    public size_restoring_window<QMainWindow>,
    public message_handler,
    public view::users::observer,
    public view::rooms::observer
{
    Q_OBJECT

    typedef size_restoring_window<QMainWindow> super;

    QMenu *_menu_file;
    QMenu *_menu_edit;
    QMenu *_menu_player;
    QMenu *_menu_comp;
    QMenu *_menu_dloads;
    QMenu *_menu_help;

    QAction *_action_quit;
    QAction *_action_settings;
    QAction *_action_status;

    QActionGroup *_actiongroup_status;
    QAction *_action_chatting;
    QAction *_action_playing;
    QAction *_action_away;
    QAction *_action_donotdisturb;

    //QAction *_action_rvr;
    QAction *_action_rvio_online;
    QAction *_action_rvr_home;
    QAction *_action_rvr_best3_laps;
    QAction *_action_rvr_best4_laps;
    QAction *_action_rvr_month_tracks;

    QAction *_action_dload_rvgl;
    QAction *_action_dload_rv12;
    QAction *_action_dload_rvzone;
    QAction *_action_dload_rvio;
    QAction *_action_dload_xtg;
    QAction *_action_dload_jigebren;

    QAction *_action_routerfw_help;
    QAction *_action_rvh_faq;
    QAction *_action_rv_wiki;
    QAction *_action_trh_forum;
    //QAction *_action_pub_forum;
    //QAction *_action_rvl_forum;
    QAction *_action_about_rvh;

    QAction *_action_create_room;
    QAction *_action_join_room;
    QAction *_action_refresh;
    QAction *_action_cancel;

    view::chat  *_chat_view;
    view::users *_users_view;
    view::rooms *_rooms_view;
    QLineEdit      *_msg_field;

    QSplitter *_chat_users_splitter;

    std::string _status_dht;
    std::vector<std::string> _status_dht_extra;
    std::string _status_ctz;
    std::string _status_tmp;

    util::flood_control _flood_control;

    networking::ip_block::store _global_ip_block;

    time_t _last_connect;
    size_t _conn_tries;

    bool _ctz_disconnected;
    int  _last_dht_status_message_id;
    bool _router_fw_help_showed;

    void _create_actions();
    void _create_menus();
    void _create_toolbars();
    void _create_widgets();
    void _create_layout();
    void _connect_signals();

    bool _chat_command(const QString &t);
    void _chat_command_model_rooms();
    void _chat_command_model_users();
    void _chat_command_model_self();

    void _check_if_self_blocked();

    void _change_user_status(chat_gaming::user::status_enum user_status);

    void _update_status();
    void _room_buttons_status();

protected:
    bool handle_dht_message(::message *msg);
    bool handle_ctz_message(::message *msg);
    bool handle_external_ip_message(::message *msg);
    void handle_global_block_users(::message_block_users *msg);

public:
    house();
    virtual ~house();

    void net_connect();
    void net_disconnect();

    void handle_message(::message *msg);
    inline void status_message(const std::string &msg) {
        _chat_view->status_message(msg);
    }

    void interruptable_action_update();
    void interruptable_action_update(const std::string &s);

    // view::users::observer interface  
    virtual void user_added  (const chat_gaming::user &u);
    virtual void user_removed(const chat_gaming::user &u);
    virtual void user_blocked(const std::string &display_id);

    // view::rooms::observer interface
    virtual void room_added  (const chat_gaming::room &r);
    virtual void room_removed(const chat_gaming::room &r);

public slots:
    void show();
    void interrupt();
    void refresh();
    void enable_refresh();
    void reconnect();
    void dht_disconnected();
    void send_message();
    void room_create();
    void room_join_selected();
    void room_join_selected(QTreeWidgetItem *item, int column);
    void open_private_room (QTreeWidgetItem *item, int column);
    void open_settings();
    void open_url(const QString &url);
    void open_about();
    void change_user_status_to_chatting();
    void change_user_status_to_playing();
    void change_user_status_to_away();
    void change_user_status_to_dont_disturb();

};

} // ns window
} // ns gui

#endif //HOUSE_WINDOW_H_
