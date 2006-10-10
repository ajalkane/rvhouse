#include <utility>

#include <fx.h>
#include <fxkeys.h>

#include "../../main.h"
#include "../../messaging/message_room.h"
#include "../../messaging/message_user.h"
#include "../../util.h"
#include "../../validation.h"
#include "../../chat_gaming/room.h"
#include "../../chat_gaming/user.h"
#include "../../model/house.h"
#include "../../model/self.h"
#include "../../win_registry.h"

#include "../house_app.h"
#include "../util/util.h"

#include "room_settings.h"

namespace gui {
namespace window {

FXDEFMAP(room_settings) room_settings_map[]= {
    FXMAPFUNC(SEL_COMMAND, 
              room_settings::ID_MY_ACCEPT,
              room_settings::on_command),
    FXMAPFUNC(SEL_COMMAND, 
              room_settings::ID_MY_CANCEL,
              room_settings::on_command)
              
};

FXIMPLEMENT(room_settings, FXDialogBox, room_settings_map, ARRAYNUMBER(room_settings_map));
// FXIMPLEMENT(room_settings, FXDialogBox, NULL, 0);

room_settings::room_settings(FXWindow *owner) 
    : FXDialogBox(owner, langstr("room_settings_win/title"))
{
    FXMatrix *m = new FXMatrix(this, 2, MATRIX_BY_COLUMNS);
    
    new FXLabel(m, langstr("room_settings_win/topic"));
    _topic_field = new FXTextField(m, 25);
    new FXLabel(m, langstr("room_settings_win/password"));
    _pass_field  = new FXTextField(m, 25, NULL, 0,
                                   TEXTFIELD_PASSWD|FRAME_SUNKEN|FRAME_THICK);
        
    new FXSeparator(this);
    _pickups_check = new FXCheckButton(this, langstr("room_settings_win/pickups"));
    m = new FXMatrix(this, 2, MATRIX_BY_COLUMNS);
    new FXLabel(m, langstr("room_settings_win/laps"));
    _laps_field = new FXSpinner(m,3,NULL,0,SPIN_NORMAL|FRAME_SUNKEN|FRAME_THICK);
    _laps_field->setRange(1, 20);
    new FXLabel(m, langstr("room_settings_win/players"));
    _players_field = new FXSpinner(m,3,NULL,0,SPIN_NORMAL|FRAME_SUNKEN|FRAME_THICK);
    _players_field->setRange(2, 12);

    new FXSeparator(this);  
    FXHorizontalFrame *bframe = new FXHorizontalFrame(this, LAYOUT_CENTER_X);

    util::create_default_button(
        bframe, langstr("common/ok_button"), this, ID_MY_ACCEPT
    );
    util::create_button(
        bframe, langstr("common/cancel_button"), this, ID_MY_CANCEL
    );
  /*
    new FXButton(
        closebox,langstr("common/ok_button"),
        NULL,this,FXTopWindow::ID_CLOSE,
        BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_RIGHT|
        FRAME_RAISED|FRAME_THICK,
        0,0,0,0,20,20
    );
    new FXButton(
        closebox,langstr("common/cancel_button"),
        NULL,this,FXTopWindow::ID_CLOSE,
        BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_RIGHT|
        FRAME_RAISED|FRAME_THICK,
        0,0,0,0,20,20
    );
    
    new FXButton(bframe, langstr("common/ok_button"), NULL, this, ID_MY_ACCEPT);
    new FXButton(bframe, langstr("common/cancel_button"), NULL, this, ID_MY_CANCEL);
    */
    /* Room initialization, settings taken from existing room.*/    
    chat_gaming::room &r = self_model()->hosting_room();
    _registry_to_room(r);
    _room_id_prev = r.id();
    _room_to_form(r);
    if (_room_id_prev.empty()) {
        // New room branch
        r.owner_id(self_model()->user().id());
        r.generate_id();
    
        // Set the user to be in this room and send an update
        self_model()->user().room_id(r.id());
        self_model()->user_send();
    }
    
    getAccelTable()->addAccel(MKUINT(KEY_F4,ALTMASK),this,FXSEL(SEL_COMMAND,ID_MY_CANCEL));     
}

void
room_settings::create() {
    FXDialogBox::create();
}

void
room_settings::_room_to_form(const chat_gaming::room &r)
{
    _topic_field->setText(r.topic().c_str());
    _pass_field->setText(r.password().c_str());
    _laps_field->setValue(r.laps());
    _players_field->setValue(r.max_players());
    _pickups_check->setCheck(r.pickups());
}

void
room_settings::_form_to_room(chat_gaming::room &r) const
{
    r.topic(_topic_field->getText().trim().text());
    r.password(_pass_field->getText().text());
    r.laps(_laps_field->getValue());
    r.max_players(_players_field->getValue());
    r.pickups(_pickups_check->getCheck());
}

void
room_settings::_registry_to_room(chat_gaming::room &r) const
{
    r.laps       (game_registry()->get<int>("NLaps", 10));
    r.max_players(game_registry()->get<int>("NCars", 10));
    r.pickups    (game_registry()->get<int>("Pickups", 0));
}

void
room_settings::_room_to_registry(const chat_gaming::room &r) const
{
    game_registry()->set("NLaps",   r.laps());
    game_registry()->set("NCars",   r.max_players());
    game_registry()->set("Pickups", r.pickups() ? 1 : 0);
}

long 
room_settings::on_command(FXObject *from, FXSelector sel, void *ptr) {
    ::message *msg = NULL;
    FXSelector nsel = 0;
    
    switch (FXSELID(sel)) {
    case ID_MY_ACCEPT:
    {
        msg = _room_message();
        if (msg) nsel = FXSEL(FXSELTYPE(sel), ID_ACCEPT);
        break;      
    }
    case ID_MY_CANCEL:
        // Set the user to be in the room he was before the call
        // (this done only when new room)
        if (_room_id_prev != self_model()->hosting_room().id()) {
            self_model()->user().room_id(_room_id_prev);
            self_model()->hosting_room().id(chat_gaming::room::id_type());
            msg = self_model()->user_as_message();
        }
        nsel = FXSEL(FXSELTYPE(sel), ID_CANCEL);
        break;
    }

    if (msg) net_messenger()->send_msg(msg);
    
    if (nsel) {
        this->handle(from, nsel, ptr);
    }
    return 1;       
}

::message *
room_settings::_room_message() {
    if (_topic_field->getText().trim().empty()) {
        FXMessageBox::error(this, FX::MBOX_OK, 
                            langstr("room_settings_win/error_title"),
                            langstr("room_settings_win/topic_missing"));
        return NULL;
    }

    _form_to_room(self_model()->hosting_room());
    _room_to_registry(self_model()->hosting_room());
    
    return self_model()->hosting_room_as_message();
#if 0
    new message_room(
        ::message::room, 
        self_model()->hosting_room(), 
        self_model()->sequence(),
        0
    );
#endif
    //                         _user_field->getText().text(),
    //                        _pass_field->getText().text());
}

} // ns window
} // ns gui
