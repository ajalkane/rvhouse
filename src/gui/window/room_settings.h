#ifndef _ROOM_SETTINGS_WINDOW_H_
#define _ROOM_SETTINGS_WINDOW_H_

#include <string>

#include <fx.h>

#include "../../common.h"
#include "../../messaging/messenger.h"
#include "../../messaging/message.h"
#include "../../messaging/message_string.h"
#include "../../chat_gaming/room.h"

#include "../message_handler.h"

namespace gui {
namespace window {

class room_settings : public FXDialogBox {
    FXDECLARE(room_settings)

    FXTextField *_topic_field;
    FXTextField *_pass_field;
    FXSpinner   *_laps_field;
    FXSpinner   *_players_field;
    FXCheckButton *_pickups_check;
    
    FXHorizontalFrame *_toolbar;
    
    chat_gaming::room::id_type _room_id_prev;
    
    void _room_to_form(const chat_gaming::room &r);
    void _form_to_room(chat_gaming::room &r) const;
    void _registry_to_room(chat_gaming::room &r) const;
    void _room_to_registry(const chat_gaming::room &r) const;

    ::message *_room_message();

protected:
    room_settings() {}

public:
    enum {
        ID_MY_ACCEPT = FXDialogBox::ID_LAST,
        ID_MY_CANCEL,
    };

    room_settings(FXWindow *owner);
    virtual void create();
        
    long on_command(FXObject *from, FXSelector sel, void *);    
};

} // ns window
} // ns gui

#endif //_ROOM_SETTINGS_WINDOW_H_
