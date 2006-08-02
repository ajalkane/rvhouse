#ifndef _HOUSE_WINDOW_H_
#define _HOUSE_WINDOW_H_

#include <string>
#include <vector>
#include <fx.h>

#include <ace/Time_Value.h>
#include <time.h>

#include "../../common.h"
#include "../../messaging/messenger.h"
#include "../../messaging/message.h"
#include "../../messaging/message_string.h"
#include "../../multi_feed/user_item.h"
#include "../view/chat.h"
#include "../view/users.h"
#include "../view/rooms.h"
#include "../message_handler.h"
#include "../watched_window.h"

namespace gui {
namespace window {

class house
            : public FXMainWindow,
            public message_handler,
            public watched_window,
            public view::users::observer,
            public view::rooms::observer
{
    FXDECLARE(house)

    FXHorizontalFrame *_toolbar;
    // FXLabel       *_status;
    FXStatusBar       *_status;
    FXTextField       *_msg_field;
    FXButton          *_interrupt_button;
    FXButton          *_room_create_button;
    FXButton          *_room_join_button;
    FXButton          *_refresh_button;
    view::chat        *_chat_view;
    view::users       *_users_view;
    view::rooms       *_rooms_view;

    std::string _status_dht;
    std::vector<std::string> _status_dht_extra;
    std::string _status_ctz;
    std::string _status_tmp;

    time_t _last_connect;
    ACE_Time_Value _flood_last_sent_message;
    int    _flood_control;
    size_t _conn_tries;

    bool _ctz_disconnected;
    int  _last_dht_status_message_id;

    void _update_status();
    void _room_buttons_status();
    bool _chat_command(const FXString &t);
    void _chat_command_model_rooms();
    void _chat_command_model_users();
    void _chat_command_model_self();
protected:
    house() {}

    bool handle_dht_message(::message *msg);
    bool handle_ctz_message(::message *msg);
    bool handle_external_ip_message(::message *msg);
public:
    enum {
        ID_CONNECT = FXMainWindow::ID_LAST,
        ID_DISCONNECT,
        ID_SEND_MSG,
        ID_ROOM_CREATE,
        ID_ROOM_JOIN,
        ID_REFRESH,
        ID_REFRESH_ENABLE,
        ID_INTERRUPT,
        ID_ABOUT,
        ID_RECONNECT,
        ID_DHT_DISCONNECTED,
        ID_LAST,
    };

    house(FXApp *a);
    ~house();
    virtual void create();

    void interruptable_action_update();
    void interruptable_action_update(const std::string &s);

    long on_network_command (FXObject *from, FXSelector sel, void *);
    long on_send_message    (FXObject *from, FXSelector sel, void *);
    long on_room_create     (FXObject *from, FXSelector sel, void *);
    long on_room_join       (FXObject *from, FXSelector sel, void *);
    long on_interrupt       (FXObject *from, FXSelector sel, void *);
    long on_refresh         (FXObject *from, FXSelector sel, void *);
    long on_refresh_enable  (FXObject *from, FXSelector sel, void *);
    long on_reconnect       (FXObject *from, FXSelector sel, void *);
    long on_about           (FXObject *from, FXSelector sel, void *);
    long on_dht_disconnected(FXObject *from, FXSelector sel, void *);

    void handle_message    (::message *msg);

    inline void status_message(const std::string &msg) {
        _chat_view->status_message(msg);
    }

    // view::users::observer interface
    virtual void user_added(const chat_gaming::user &u);
    virtual void user_removed(const chat_gaming::user &u);

    // view::rooms::observer interface
    virtual void room_added(const chat_gaming::room &r);
    virtual void room_removed(const chat_gaming::room &r);

};

} // ns window
} // ns gui

#endif //_HOUSE_WINDOW_H_
