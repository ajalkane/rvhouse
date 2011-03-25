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
              room_settings::on_command),
    FXMAPFUNC(SEL_COMMAND,
              room_settings::ID_VERSION,
              room_settings::on_version)
              
};

FXIMPLEMENT(room_settings, FXDialogBox, room_settings_map, ARRAYNUMBER(room_settings_map));

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

    FXComposite *divide = new FXHorizontalFrame(this);
    FXComposite *column1 = new FXVerticalFrame(divide);
    FXComposite *column2 = new FXVerticalFrame(divide,FRAME_SUNKEN); //,0,0,0,0,0,0,0,0);

    _pickups_check = new FXCheckButton(column1, langstr("room_settings_win/pickups"));
    m = new FXMatrix(column1, 2, MATRIX_BY_COLUMNS);
    new FXLabel(m, langstr("room_settings_win/laps"));
    _laps_field = new FXSpinner(m,3,NULL,0,SPIN_NORMAL|FRAME_SUNKEN|FRAME_THICK);
    _laps_field->setRange(1, 20);
    new FXLabel(m, langstr("room_settings_win/players"));
    _players_field = new FXSpinner(m,3,NULL,0,SPIN_NORMAL|FRAME_SUNKEN|FRAME_THICK);
    _players_field->setRange(2, 12);

    _version_check   = new FXCheckButton(column2, langstr("room_settings_win/version"), this, ID_VERSION);
    _version_all     = new FXRadioButton(column2, langstr("room_settings_win/version_all"), this, ID_VERSION);
    _version_12_only = new FXRadioButton(column2, langstr("room_settings_win/version_12_only"), this, ID_VERSION);
    _version_check->setTipText(langstr("room_settings_win/version_tip"));
    _version_all->setTipText(langstr("room_settings_win/version_all_tip"));
    _version_12_only->setTipText(langstr("room_settings_win/version_12_tip"));

    new FXSeparator(this);

    FXHorizontalFrame *bframe = new FXHorizontalFrame(this, LAYOUT_CENTER_X);

    util::create_default_button(
        bframe, langstr("common/ok_button"), this, ID_MY_ACCEPT
    );
    util::create_button(
        bframe, langstr("common/cancel_button"), this, ID_MY_CANCEL
    );

    /* Room initialization, settings taken from existing room.*/    
    _from_settings_to_form();
    
    getAccelTable()->addAccel(MKUINT(KEY_F4,ALTMASK),this,FXSEL(SEL_COMMAND,ID_MY_CANCEL));     
}

void
room_settings::create() {
    FXDialogBox::create();
    // Initialize the version dialog
    on_version(_version_check, 0, NULL);
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
    // Some sensible limits
    if (_topic_field->getText().length() > 80)
        _topic_field->setText(_topic_field->getText().left(80));
    if (_pass_field->getText().length() > 32)
        _pass_field->setText(_pass_field->getText().left(32));
    
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
        msg = _to_settings_and_form_room_message();
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

long
room_settings::on_version(FXObject *from, FXSelector sel, void *ptr) {
    if (from == _version_check) {
        if (_version_check->getCheck()) {
            _version_all->enable();
            _version_12_only->enable();
            if (!_version_all->getCheck() && !_version_12_only->getCheck()) {
                _version_all->setCheck(TRUE);
            }
        } else {
            _version_all->disable();
            _version_all->setCheck(FALSE);
            _version_12_only->disable();
            _version_12_only->setCheck(FALSE);
        }
    }
    else if (from == _version_all) {
        if (_version_all->getCheck()) {
            _version_12_only->setCheck(FALSE);
        }
    }
    else if (from == _version_12_only) {
        if (_version_all->getCheck()) {
            _version_all->setCheck(FALSE);
        }
    }
    return 0;
}

void
room_settings::_from_settings_to_form() {
    chat_gaming::room &target_room = self_model()->hosting_room();

    _registry_to_room(target_room);
    _room_id_prev = target_room.id();
    _room_to_form(target_room);

    _version_check->setCheck(self_model()->room_version());
    _version_all->setCheck(self_model()->room_version_all());
    _version_12_only->setCheck(self_model()->room_version_12_only());

    if (_room_id_prev.empty()) {
        // New room branch
        target_room.owner_id(self_model()->user().id());
        target_room.generate_id();

        // Set the user to be in this room and send an update
        self_model()->user().room_id(target_room.id());
        self_model()->user_send();
    }
}

::message *
room_settings::_to_settings_and_form_room_message() {
    if (_topic_field->getText().trim().empty()) {
        FXMessageBox::error(this, FX::MBOX_OK, 
                            langstr("room_settings_win/error_title"),
                            langstr("room_settings_win/topic_missing"));
        return NULL;
    }

    _form_to_room(self_model()->hosting_room());
    _room_to_registry(self_model()->hosting_room());
    
    self_model()->room_version(_version_check->getCheck());
    self_model()->room_version_all(_version_all->getCheck());
    self_model()->room_version_12_only(_version_12_only->getCheck());

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
