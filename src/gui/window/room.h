#ifndef _ROOM_WINDOW_H_
#define _ROOM_WINDOW_H_

#include <string>
#include <fx.h>

#include "../../chat_gaming/house.h"
#include "../../chat_gaming/room.h"
#include "../../common.h"
#include "../view/chat.h"
#include "../view/users.h"
#include "../message_handler.h"
#include "../watched_window.h"
#include "../util/flood_control.h"

namespace gui {
namespace window {

class room 
  // : public FXMainWindow, 
  : public FXTopWindow, 
    public message_handler, 
    public watched_window,
    public view::users::observer
{
    typedef FXTopWindow super;
    FXDECLARE(room)

    FXTextField       *_msg_field;
    FXButton          *_edit_button;
    FXButton          *_launch_button;
    FXButton          *_share_button;
    FXLabel           *_info_picks;
    FXLabel           *_info_laps;
    FXLabel           *_info_players;
    
    view::chat        *_chat_view;
    view::users       *_users_view;
    bool               _running_modal;
    bool               _hosting;
    bool               _host_sharing;
    
    // chat_gaming::room _room;
    chat_gaming::room::id_type _room_id;    
    chat_gaming::user::id_type _host_id;

    util::flood_control _flood_control;
        
    void _handle_room_launch(::message *msg);
    void _handle_room_kick  (::message *msg);
    void _launcher_error(int err);
    void _launch_host();
    void _launch_join(chat_gaming::house::user_iterator host_ui);
    void _set_rv_cmdline();
    model::house::user_iterator _playing_host();
    void _launched_display();
    void _buttons_state();
    void _init();
    void _room_message(const chat_gaming::user &u, int grp);
    void _button_sharing_tracks_enable(bool enable_if_possible);    
protected:
    room() {}

public:
    enum {
        ID_EDIT_ROOM = FXMainWindow::ID_LAST,
        ID_SEND_MSG,
        ID_LAUNCH,
        ID_SHARE_TRACKS,
        ID_LAST,
    };
    
    room(FXApp *a,        const chat_gaming::room::id_type &id);
    room(FXWindow *owner, const chat_gaming::room::id_type &id);
    ~room();
    virtual void create();

    void update(int grp);
    
    // If this function enabled, _host_id must be reset too
    // inline void room_id(const chat_gaming::room::id_type &rid) {
    //  _room_id = rid;
    //}
    
    // inline const chat_gaming::room &room() { return _room; }
    // const chat_gaming::room &room(const chat_gaming::room &r);

    long on_edit_room(FXObject *from, FXSelector sel, void *);
    long on_send_message(FXObject *from, FXSelector sel, void *);
    long on_launch(FXObject *from, FXSelector sel, void *);
    long on_share_tracks(FXObject *from, FXSelector sel, void *);
    
    void handle_message    (::message *msg);    

    // view::users::observer interface  
    virtual void user_added(const chat_gaming::user &u);
    virtual void user_removed(const chat_gaming::user &u);
    virtual void user_blocked(const std::string &display_id);
    
    virtual void sharing_tracks_changed(const chat_gaming::user &u,
                                        bool value);
    virtual void getting_tracks_changed(const chat_gaming::user &u,
                                        bool value);
};

} // ns window
} // ns gui

#endif //_ROOM_WINDOW_H_
