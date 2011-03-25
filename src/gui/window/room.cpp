#include <ace/Process.h>

#include <utility>
#include <climits>

#include <fx.h>
#include <fxkeys.h>

#include "../../messaging/message_user.h"
#include "../../messaging/message_room.h"
#include "../../messaging/message_channel.h"
#include "../../messaging/message_send_room.h"
#include "../../messaging/message_room_command.h"
#include "../../util.h"
#include "../../os_util.h"
#include "../../config_file.h"
#include "../../model/house.h"
#include "../../model/self.h"
#include "../../executable/launcher.h"
#include "../../icon_store.h"
#include "../../rv_cmdline_builder.h"
#include "../../win_registry.h"
#include "../house_app.h"
#include "../util/util.h"
#include "room_settings.h"
#include "room.h"

namespace gui {
namespace window {

FXDEFMAP(room) room_map[]= {
  FXMAPFUNC(SEL_COMMAND,  room::ID_SEND_MSG,
                          room::on_send_message),
  FXMAPFUNC(SEL_COMMAND,  room::ID_EDIT_ROOM,
                          room::on_edit_room),
  FXMAPFUNC(SEL_COMMAND,  room::ID_SHARE_TRACKS,
                          room::on_share_tracks),
  FXMAPFUNC(SEL_COMMAND,  room::ID_LAUNCH,
                          room::on_launch),
  FXMAPFUNC(SEL_TIMEOUT,  room::ID_LAUNCH,
                          room::on_launch),
};

// FXIMPLEMENT(room, FXMainWindow, room_map, ARRAYNUMBER(room_map))
FXIMPLEMENT(room, room::super, room_map, ARRAYNUMBER(room_map))

#define STRING_JOIN(target, sep, src) \
target = (target.empty() ? src : (src.empty() ? target : target + sep + src))

// FXIMPLEMENT(room, FXMainWindow, NULL, 0)

room::room(FXApp *a, const chat_gaming::room::id_type &id)
    : super(a, "", NULL, NULL, DECOR_ALL, 0, 0, 350, 300, 0,0,0,0,0,0),
      _running_modal(false), _room_id(id)
    // : FXMainWindow(a, "", NULL, NULL, DECOR_ALL, 0, 0, 350, 300),
{
    _init();
}

room::room(FXWindow *owner, const chat_gaming::room::id_type &id)
    : super(owner, "",  NULL, NULL, DECOR_ALL, 0, 0, 350, 300, 0,0,0,0,0,0),
      _running_modal(false), _room_id(id)
    // : FXMainWindow(a, "", NULL, NULL, DECOR_ALL, 0, 0, 350, 300),
{
    _init();
}

void
room::_init() {
    ACE_DEBUG((LM_DEBUG, "room::ctor %d\n", (FXWindow *)this));

    _hosting = (_room_id == self_model()->hosting_room().id());
    _host_sharing = false;

    // Find host id of the room
    while (1) {
        model::house::room_iterator room_i
          = house_model()->room_find(_room_id);
        if (room_i == house_model()->room_end()) break;
        model::house::user_iterator host_i
          = house_model()->user_find(room_i->owner_id());
        if (host_i == house_model()->user_end()) break;
        _host_id = host_i->id();
        break;
    }

    if (_host_id == chat_gaming::user::id_type()) {
        ACE_DEBUG((LM_WARNING, "room::_init(): host_id not found for "
                   " room id %s\n", _room_id.c_str()));
    }

    // new FXToolTip(getApp());

    setIcon(app_icons()->get("rv_house"));
    setMiniIcon(app_icons()->get("rv_house"));

    int button_opts = ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED;

    // FXVerticalFrame *c = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);

   //  toolbar  = new FXHorizontalFrame(c);

    FXVerticalFrame *c = new FXVerticalFrame(
        this, LAYOUT_FILL_X|LAYOUT_FILL_Y,
        0,0,0,0, 0,0,0,0, 0,0
    );
    FXComposite *toolbarcontainer = new FXHorizontalFrame(
        c, LAYOUT_SIDE_TOP|LAYOUT_FILL_X,
        0,0,0,0, 0,0,0,0, 0,0
    );
    new FXToolBarTab(toolbarcontainer,NULL,0,FRAME_RAISED);
    FXComposite *toolbar = new FXToolBar(
        toolbarcontainer,
        FRAME_RAISED|
        LAYOUT_SIDE_TOP|LAYOUT_FILL_X,
        0,0,0,0, 4,4,4,4, 0,0
    );

    new FXButton(toolbar,
                 util::button_text(NULL, langstr("room_win/leave")),
                 app_icons()->get("close"),
                 this, ID_CLOSE, button_opts);

    _edit_button =
      new FXButton(toolbar,
                   util::button_text(NULL, langstr("room_win/settings")),
                   app_icons()->get("settings"), this, ID_EDIT_ROOM,
                   button_opts);
    _launch_button =
      new FXButton(toolbar,
                   util::button_text(NULL, langstr("room_win/launch")),
                   app_icons()->get("launch"), this, ID_LAUNCH,
                   button_opts);

    new FXVerticalSeparator(toolbar);

    std::string share_btn_text("\t");
    share_btn_text += (_hosting
                       ? langstr("room_win/share_tracks")
                       : langstr("room_win/dload_tracks"));
    _share_button  = new FXButton(toolbar, share_btn_text.c_str(),
                     app_icons()->get("tracks_share"), this, ID_SHARE_TRACKS,
                     button_opts);
    // NOTE: the external track sharing application (RVTM) does not seem to work,
    // and don't seem to be fixed. So for now hide the track sharing to not confuse
    _share_button->hide();

    new FXVerticalSeparator(toolbar);

    FXComposite *infoframe =
     new FXMatrix(
        toolbar, 2, MATRIX_BY_COLUMNS,
        0,0,0,0,
        DEFAULT_SPACING,
        DEFAULT_SPACING,
        0,0,15,0
        );
    new FXLabel(infoframe, langstr("words/players"),NULL,LABEL_NORMAL,0,0,0,0,0,0,0,0);
    _info_players =
    new FXLabel(infoframe, "",NULL,LABEL_NORMAL,0,0,0,0,0,0,0,0);
    new FXLabel(infoframe, langstr("words/laps"),NULL,LABEL_NORMAL,0,0,0,0,0,0,0,0);
    _info_laps    =
    new FXLabel(infoframe, "",NULL,LABEL_NORMAL,0,0,0,0,0,0,0,0);
    new FXLabel(infoframe, langstr("words/pickups"),NULL,LABEL_NORMAL,0,0,0,0,0,0,0,0);
    _info_picks   =
    new FXLabel(infoframe, "",NULL,LABEL_NORMAL,0,0,0,0,0,0,0,0);

    // If not host, disable Edit and Launch buttons
    if (!_hosting) {
        _edit_button->disable();
        _launch_button->disable();
        _share_button->disable();
    }
    FXSplitter *house     = new FXSplitter(c, SPLITTER_HORIZONTAL |
                                                 SPLITTER_REVERSED   |
                                                 LAYOUT_FILL_X |
                                                 LAYOUT_FILL_Y);
    FXSplitter *sections  = new FXSplitter(house, LAYOUT_FILL_X |
                                                  LAYOUT_FILL_Y |
                                                  SPLITTER_VERTICAL);
    _users_view = new view::users(util::framed_container(house), NULL); // , 0, FRAME_SUNKEN|FRAME_THICK);
    _users_view->room_id(_room_id);

    // new FXFrame(_users_view);
    // new FXLabel(house, "Users View", NULL, FRAME_SUNKEN|FRAME_THICK);
    // _rooms_view = new rooms_view(sections);
    // new FXLabel(sections, "Rooms View", NULL, FRAME_SUNKEN|FRAME_THICK);
    _chat_view = new view::chat(util::framed_container(sections));
    _chat_view->channel(_room_id);

    // new FXLabel(sections, "Chat Window View", NULL, FRAME_SUNKEN|FRAME_THICK);
    FXVerticalFrame *b = new FXVerticalFrame(
        c, LAYOUT_FILL_X,
        0,0,0,0,
        0,0);

    _msg_field = new FXTextField(b, 0,  this, ID_SEND_MSG,
                                 FRAME_SUNKEN|FRAME_THICK|
                                 LAYOUT_FILL_X|TEXTFIELD_ENTER_ONLY);
    _msg_field->setFocus();
    // _status->getStatusLine()->setNormalText("Waiting...");

    // For some reason, restore_size has to be in constructor
    // if FXMainWindow and in create if FXDialogBox.
    util::restore_size(this, "room_win");

    house->setSplit(1, 150);

    _users_view->setWidth(150);
    _users_view->observer_set(this);

    getAccelTable()->addAccel(MKUINT(KEY_F4,ALTMASK),this,FXSEL(SEL_COMMAND,ID_CLOSE));
}

room::~room() {
    ACE_DEBUG((LM_DEBUG, "room::dtor\n"));

    util::store_size(this, "room_win");

    // Set ourself to be in no room anymore, and off we go
    self_model()->user().room_id(chat_gaming::room::id_type());
    self_model()->hosting_room().id(chat_gaming::room::id_type());
    self_model()->user_send();
    getApp()->removeTimeout(this, ID_LAUNCH);

    delete _users_view;
    delete _chat_view;

    ACE_DEBUG((LM_DEBUG, "room: dtor done\n"));
}

void
room::create() {
    super::create();
    watched_window::create(this);
    // show(PLACEMENT_SCREEN);
    show();

    // util::restore_size(this, "room", "win");

    _buttons_state();

    if (_hosting &&
        pref()->get("general", "send_ip", true) &&
        pref()->get("one_time_alert", "send_ip", true))
    {
        pref()->set("one_time_alert", "send_ip", false);
        const char *topic   = langstr("room_win/send_ip_info_title");
        const char *content = langstr("room_win/send_ip_info");

        variable_guard<bool> guard(_running_modal); _running_modal = true;
        FXMessageBox::information(this, FX::MBOX_OK, topic, content);
    }

}

void
room::update(int grp) {
    model::house::house_type::room_iterator r =
      house_model()->room_find(_room_id, grp);

    if (r == house_model()->room_end()) {
        ACE_DEBUG((LM_ERROR, "room: no matching room found "
                  "for id %s from group %d\n", _room_id.c_str(), grp));
        return;
    }
    ACE_DEBUG((LM_DEBUG, "room: setting title to %s\n",
               r->topic().c_str()));
    setTitle(r->topic().c_str());

    _info_picks->setText(
        r->pickups()
        ? langstr("words/yes")
        : langstr("words/no")
    );
    std::ostringstream conv_plrs;
    std::ostringstream conv_laps;
    conv_plrs << r->max_players();
    conv_laps << r->laps();
    _info_players->setText(conv_plrs.str().c_str());
    _info_laps   ->setText(conv_laps.str().c_str());
}

long
room::on_send_message(FXObject *from, FXSelector sel, void *) {
    FXString t = _msg_field->getText();

    if (t.empty()) return 1;

    if (!_flood_control.allow_send(t)) {
        _chat_view->status_message(langstr("chat/flood_control"));
        return 0;
    }

    if (t.length() > (int)app_opts.limit_chat_msg())
        t.trunc(app_opts.limit_chat_msg());
    t.substitute("\r", "");

    // Sending a room message
    message_send_room *msg =
      new message_send_room(::message::send_room,
                            t.text(),
                            self_model()->user().id(),
                            _room_id,
                            self_model()->sequence(),

                            0);
    net_messenger()->send_msg(msg);

    _msg_field->setText("");

    return 1;
}

long
room::on_edit_room(FXObject *from, FXSelector sel, void *) {
    if (_room_id == self_model()->hosting_room().id()) {
        window::room_settings *win = new window::room_settings(this);
        if (!win->execute(PLACEMENT_SCREEN)) {
            ACE_DEBUG((LM_DEBUG, "room::Room create cancelled\n"));
            return 1;
            // handle(this, FXSEL(SEL_COMMAND, ID_QUIT), NULL);
        }
    }

    return 0;
}

long
room::on_launch(FXObject *from, FXSelector sel, void *) {
    rv_cmdline_builder cmdline_builder;
    _set_room_cmdline(cmdline_builder);
    cmdline_builder.set_rv_cmdline();

    if (_room_id == self_model()->hosting_room().id()) {
        _launch_host();
    } else {
        model::house::user_iterator playing_host = _playing_host();

        if (playing_host != house_model()->user_end()) {
            // Check how long the host has been playing and don't
            // allow join if too long played
            int rejoin_time = conf()->get<int>("play", "rejoin_time", INT_MAX);
            ACE_DEBUG((LM_DEBUG, "room::on_launch "
                      "rejoin_time/time/statustime/dif: %d/%d/%d/%d\n",
                      rejoin_time, time(NULL), playing_host->status_time(),
                      time(NULL) - playing_host->status_time()));
            if (time(NULL) - playing_host->status_time() > rejoin_time) {
                std::ostringstream mins;
                mins << rejoin_time / 60;
                const char *topic = langstr("room_win/join_too_late_title");
                std::string content = langstr("room_win/join_too_late",
                                              mins.str().c_str());
                ACE_DEBUG((LM_DEBUG, "room::on_launch "
                          "mins: %s\n", mins.str().c_str()));
                ACE_DEBUG((LM_DEBUG, "room::on_launch "
                          "content: %s\n", content.c_str()));
                ACE_DEBUG((LM_DEBUG, "room::on_launch "
                          "raw: %s\n", langstr("room_win/join_too_late")));

                FXMessageBox::information(
                    this, FX::MBOX_OK, topic, content.c_str()
                );
            } else {
                _launch_join(playing_host);
            }
        }
    }

    return 1;
}

void
room::_set_room_cmdline(rv_cmdline_builder &builder) {
    if (self_model()->room_version()) {
        if (self_model()->room_version_all()) {
            ACE_DEBUG((LM_DEBUG, "room::setting RV 1.2 compatibility mode ON\n"));
            builder.add_option(rv_cmdline_builder::rv12_version_all);
        } else if (self_model()->room_version_12_only()) {
            ACE_DEBUG((LM_DEBUG, "room::setting RV 1.2 compatibility ONLY 1.2\n"));
            builder.add_option(rv_cmdline_builder::rv12_version_12_only);
        }
    }
}

long
room::on_share_tracks(FXObject *from, FXSelector sel, void *) {
    if (_hosting) {
        ::app()->launch_rvtm();
    } else {
        model::house::user_iterator host_ui = house_model()->user_find(_host_id);

        if (host_ui != house_model()->user_end()) {
            ::app()->launch_rvtm(host_ui->ip_as_string());
        }
    }

    return 1;
}

void
room::_launcher_error(int err) {
    const char *topic   = langstr("rv_launch/title");
    const char *content = "Could not launch";
    switch(err) {
    case executable::launcher::err_app_not_found:
        content = langstr("rv_launch/not_found");
        break;
    case executable::launcher::err_could_not_launch:
    default:
        content = langstr("rv_launch/other");
    }

    variable_guard<bool> guard(_running_modal); _running_modal = true;
    FXMessageBox::error(this, FX::MBOX_OK, topic, content);
}

void
room::handle_message(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "room::handle_message\n"));

    _users_view->handle_message(msg);
    _chat_view->handle_message(msg);

    message_grouped *mg = dynamic_cast<message_grouped *>(msg);
    if (!mg) return;

    bool do_close = false;

    switch (msg->id()) {
    case ::message::room:
    {
        message_room *m = dynamic_ptr_cast<message_room>(msg);

        if (m->room().id() == _room_id) {
            ACE_DEBUG((LM_DEBUG, "room: update received for the room\n"));
            update(m->group_base());
        }
        break;
    }
    case ::message::room_remove:
    {
        message_room *m = dynamic_ptr_cast<message_room>(msg);

        if (m->room().id() == _room_id) {
            ACE_DEBUG((LM_DEBUG, "room: close received "
                      "for this room, group %d\n", mg->group_base()));
            // Only close if room is not open in any groups ie. not found
            if (house_model()->room_find(_room_id) == house_model()->room_end())
                do_close = true;
            else
                ACE_DEBUG((LM_DEBUG, "room: close received "
                          "but room still open in some group\n"));
        }
    }
        break;
    case ::message::room_launch:
        _handle_room_launch(msg);
        break;
    case ::message::room_kick:
        _handle_room_kick(msg);
        break;
    case ::message::user:
    {
        message_user *m = dynamic_ptr_cast<message_user>(msg);
        if (m->user().room_id() == _room_id) {
            // _room_message(m->user(), m->group_base());
             _buttons_state();
        }
        // Check if for some reason self user has changed room without
        // first leaving this room (basically shouldn't happen, this just
        // to make sure no ambiguous states arise)
        // if (self_model()->user().room_id() != _room_id) {
        //  do_close = true;
        //}
    }
        break;
    case ::message::send:
        // TODO this don't work correctly yet

        ACE_DEBUG((LM_DEBUG, "window::room: msg_send: flash_room_chat: %d\n",
                  app_opts.flash_room_chat()));
        if (app_opts.flash_room_chat()) {
            message_channel *m = dynamic_ptr_cast<message_channel>(msg);
            ACE_DEBUG((LM_DEBUG,
                "window::room: msg_send channel/chat_view_chn: %s/%s\n",
                m->channel().c_str(), _chat_view->channel().c_str()));
            if (m->channel() == _chat_view->channel()) {
                ACE_DEBUG((LM_DEBUG,
                    "window::room: msg_send flashing\n"));
                os::flash_window(this->getOwner());
            }
        }
        break;
    default:
        return;
    }

    if (do_close) {
        // Refuse closing of window if a modal window is running
        // on this window! Doing that would make the application
        // confused.
        if (!_running_modal) {
            this->close();
        }
    }
}

void
room::_handle_room_launch(::message *msg) {
    // Don't do if already launching or if self is the host of this room!
    if (_running_modal || _room_id == self_model()->hosting_room().id()) return;
    message_room_command *m = dynamic_ptr_cast<message_room_command>(msg);

    // Check that the sender is the host of the room
    model::house::house_type::user_iterator ui
      = house_model()->user_find(m->sender_id(), m->group_base());
    if (ui == house_model()->user_end()) {
        ACE_DEBUG((LM_ERROR, "room::_handle_room_launch no user found "
        "for id %s\n", m->sender_id().c_str()));
        return;
    }

    model::house::house_type::room_iterator ri
      = house_model()->room_find(_room_id, m->group_base());
    if (ri == house_model()->room_end()) {
        ACE_DEBUG((LM_ERROR, "room::_handle_room_launch no room found "
        "for id %s\n", _room_id.c_str()));
        return;
    }

    if (ri->owner_id() != ui->id()) {
        ACE_DEBUG((LM_ERROR, "room::_handle_room_launch senders user id "
        "'%s' does not match room host's user id '%s'!",
        ri->owner_id().c_str(), ui->id().c_str()));
        return;
    }

    _chat_view->status_message(langstr("room_win/host_launched"));
    getApp()->addTimeout(this, ID_LAUNCH, 5000, NULL);
}

void
room::_handle_room_kick(::message *msg) {
    message_room_command *m = dynamic_ptr_cast<message_room_command>(msg);

    ACE_DEBUG((LM_DEBUG, "room::_handle_room_kick: "
              "target id/self id: %s/%s\n",
              m->target_user_id().id_str().c_str(),
              self_model()->user().id().id_str().c_str()));

    model::house::user_iterator ui_kicked
      = house_model()->user_find(m->target_user_id(), m->group_base());
    model::house::user_iterator ui_kicker
      = house_model()->user_find(m->sender_id(), m->group_base());
    model::house::room_iterator ri
      = house_model()->room_find(_room_id, m->group_base());

    if (ui_kicked == house_model()->user_end() ||
        ui_kicker == house_model()->user_end() ||
        ri        == house_model()->room_end()) {
        return;
    }

    if (ui_kicker->id() != ri->owner_id()) {
        ACE_DEBUG((LM_WARNING, "room::_handle_room kick: "
        "received kick from '%s', but room owner is '%s', ignored\n",
        ui_kicker->id().c_str(), ri->owner_id().c_str()));
        return;
    }

    if (ui_kicked->id() == ri->owner_id()) {
        ::app()->status_message(langstr("chat/host_kicked_host", ri->topic().c_str()));
    }
    else if (ui_kicked->id().id_str() != self_model()->user().id().id_str()) {
        _chat_view->status_message(
            langstr("chat/host_kicks_user", ui_kicked->display_id().c_str())
        );
        // content = "Kicking someone else";
    } else {
        ::app()->status_message(langstr("chat/host_kicks_you", ri->topic().c_str()));
        // content = "Kicking me!";
    }

    if (ui_kicked->id().id_str() == self_model()->user().id().id_str()) {
        // Close room if been kicked out... TODO, as can be seen
        // this can't be done when running modal...
        if (!_running_modal) {
            this->close();
        }
    }
}

void
room::_launch_host() {
    if (conf()->get<bool>("play", "no_launch", false) == false) {
        int ret = launcher_game()->start_host();
        if (ret) {
            _launcher_error(ret);
            return;
        }
    }
    // Send a message to room participants to start the game
    net_messenger()->send_msg(
      new message_room_command(
        ::message::room_launch,
        _room_id,
        chat_gaming::user::id_type(),
        self_model()->user().id(),
        self_model()->sequence(),
        0
      )
    );

    _launched_display();
}

void
room::_launch_join(chat_gaming::house::user_iterator host_ui) {
    if (conf()->get<bool>("play", "no_launch", false) == false) {
        int ret = launcher_game()->start_client(host_ui->ip_as_string());
        if (ret) {
            _launcher_error(ret);
            return;
        }
    }
    _launched_display();
}

void
room::_launched_display() {
    self_model()->user().status(chat_gaming::user::status_playing);
    self_model()->user_send();

    const char *topic   = langstr("rv_launch/title");
    const char *content = langstr("rv_launch/text");

    variable_guard<bool> guard(_running_modal); _running_modal = true;
    FXMessageBox::information(this, FX::MBOX_OK, topic, content);

    self_model()->user().status(chat_gaming::user::status_chatting);
    self_model()->user_send();
}

void
room::_buttons_state() {
    ACE_DEBUG((LM_DEBUG, "room::_buttons_state\n"));

    // If host rest of the buttons are always visible, so return
    if (_hosting) {
        return;
    }
    // Otherwise... if room host's status equals playing in
    // one of the feeds, then display. TODO better and
    // more general way needed here... perhaps to multi_feed
    // namespace.
    model::house::user_iterator playing_host = _playing_host();

    if (playing_host != house_model()->user_end()) {
        ACE_DEBUG((LM_DEBUG, "room::_buttons_state: showing launch button\n"));
        _launch_button->enable();
    } else {
        ACE_DEBUG((LM_DEBUG, "room::_buttons_state: hiding launch button\n"));
        _launch_button->disable();
    }
}


model::house::user_iterator
room::_playing_host() {
    const model::house::group_desc_type &g = house_model()->group_desc();
    model::house::group_desc_type::const_iterator gi = g.begin();
    for (; gi != g.end(); gi++) {
        ACE_DEBUG((LM_DEBUG, "room::_playing_host: finding room\n"));
        model::house::room_iterator room_i
          = house_model()->room_find(_room_id, gi->first);
        if (room_i == house_model()->room_end()) continue;
        ACE_DEBUG((LM_DEBUG, "room::_playing_host: finding host\n"));
        model::house::user_iterator host_i
          = house_model()->user_find(room_i->owner_id(), gi->first);
        if (host_i == house_model()->user_end()) continue;
        ACE_DEBUG((LM_DEBUG, "room::_playing_host: "
                   "host status/group: %s/%d\n",
                   host_i->status_as_string(), gi->first));
        if (host_i->status() == chat_gaming::user::status_playing) {
            return host_i;
        }
    }

    return house_model()->user_end();
}

void
room::user_added(const chat_gaming::user &u) {
    _chat_view->status_message(
        langstr("chat/user_joined_room", u.display_id().c_str())
    );


    ACE_DEBUG((LM_DEBUG, "room::user_added: flash_new_user_in_room: %d, user_status: %d\n",
               app_opts.flash_new_user_in_room(), self_model()->user().status()));

// TODO: this does not work, probably because room window is always on top
// of main window
//    if (app_opts.flash_new_user_in_room() &&
//        self_model()->user().status() != chat_gaming::user::status_playing)
//    {
//        os::flash_window(this);
//    }

    ACE_DEBUG((LM_DEBUG, "room::user_added: user id/self id: %s/%s\n",
               u.id().id_str().c_str(), self_model()->user().id().id_str().c_str()));

    if (_hosting
        && pref()->get("general", "send_ip", true)
        && u.id().id_str() != self_model()->user().id().id_str()) {
        // Send our IP to the new joiner. The channel used
        // is this particular room, which will cause the
        // user to receive the message in this room.
        std::string external_ip = self_model()->user().ip_as_string();
        if (external_ip.empty()) external_ip = "<undetected>";
        message_channel *msg =
          new message_channel(::message::send_notification,
                              langstr("chat/host_ip", external_ip.c_str()),
                              u.id(),
                              _room_id,
                              self_model()->sequence(),
                              0);
        net_messenger()->send_msg(msg);
    }
    if (u.sharing_tracks()) sharing_tracks_changed(u, true);
    if (u.getting_tracks()) getting_tracks_changed(u, true);
}
void
room::user_removed(const chat_gaming::user &u) {
    _chat_view->status_message(
        langstr("chat/user_exited_room", u.display_id().c_str())
    );
}

void
room::user_blocked(const std::string &display_id) {
    _chat_view->status_message(langstr("chat/user_ignored",display_id.c_str()));
}

void
room::sharing_tracks_changed(const chat_gaming::user &u,
                                    bool value)
{
    std::string msg =
        value ? langstr("chat/user_shares_tracks", u.display_id().c_str())
              : langstr("chat/user_shares_tracks_stops", u.display_id().c_str());

    _chat_view->status_message(msg);

    ACE_DEBUG((LM_DEBUG, "room::sharing_tracks_changed: user/host: %s/%s\n",
               u.id().id_str().c_str(), _host_id.id_str().c_str()));

    if (u.id().id_str() == _host_id.id_str()) {
        _host_sharing = value;
        bool allow_share_enable = false;
        if (value) {
            allow_share_enable = _hosting ? false : true;
        } else {
            allow_share_enable = _hosting ? true : false;
        }

        _button_sharing_tracks_enable(allow_share_enable);
    } else if (u.id().id_str() == self_model()->user().id().id_str()) {
        _button_sharing_tracks_enable(_host_sharing);
    }
}

void
room::getting_tracks_changed(const chat_gaming::user &u,
                                    bool value)
{
    std::string msg =
        value ? langstr("chat/user_dloads_tracks", u.display_id().c_str())
              : langstr("chat/user_dloads_tracks_stops", u.display_id().c_str());

    _chat_view->status_message(msg);

    if (!_hosting ||
        u.id().id_str() == self_model()->user().id().id_str()) {
        _button_sharing_tracks_enable(_host_sharing ? true : false);
    }
}

void
room::_button_sharing_tracks_enable(bool enable_if_possible) {
    if (enable_if_possible &&
        !self_model()->user().sharing_tracks() &&
        !self_model()->user().getting_tracks())
        _share_button->enable();
    else
        _share_button->disable();
}

} // ns window
} // ns gui
