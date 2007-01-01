#ifndef _PRIVATE_MESSAGE_WINDOW_H_
#define _PRIVATE_MESSAGE_WINDOW_H_

#include <string>
#include <bitset>
#include <fx.h>

#include "../../chat_gaming/house.h"
#include "../../chat_gaming/room.h"
#include "../../common.h"
#include "../view/chat.h"
#include "../view/users.h"
#include "../message_handler.h"
#include "../watched_window.h"
#include "../util/slots.h"
#include "../util/flood_control.h"

namespace gui {
namespace window {

class private_message 
  // : public FXMainWindow, 
  : public FXMainWindow, 
    public message_handler, 
    public watched_window,
    public view::users::observer
{
    typedef FXMainWindow super;
    FXDECLARE(private_message)

    FXHorizontalFrame *_toolbar;
    FXTextField       *_msg_field;
    
    view::chat        *_chat_view;
    view::users       *_users_view;
    
    // chat_gaming::room _room;
    std::string _channel;
    std::string _user_id_str;

    // Used to hold a slot for a free private window position
    static util::slots<16> _slots;
    util::slot             _slot;
    util::flood_control    _flood_control;
    
    void _init2();
    int  _find_free_slot();
protected:
    private_message() {}

public:
    enum {
        ID_SEND_MSG = super::ID_LAST,
        ID_LAST,
    };
    
    private_message(FXApp *a,        const std::string &user_id);
    // private_message(FXWindow *owner, const std::string &user_id);
    ~private_message();
    virtual void create();
    
    // If this function enabled, _host_id must be reset too
    // inline void room_id(const chat_gaming::room::id_type &rid) {
    //  _room_id = rid;
    //}
    
    // inline const chat_gaming::room &room() { return _room; }
    // const chat_gaming::room &room(const chat_gaming::room &r);

    inline const std::string &user_id_str() const { return _user_id_str; }
    inline const std::string &channel() const     { return _channel; }
    
    long on_send_message(FXObject *from, FXSelector sel, void *);
    
    void handle_message    (::message *msg);    

    // view::users::observer interface  
    virtual void user_added(const chat_gaming::user &u);
    virtual void user_removed(const chat_gaming::user &u);
    
};

} // ns window
} // ns gui

#endif //_PRIVATE_MESSAGE_WINDOW_H_
