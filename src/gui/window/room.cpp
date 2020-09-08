#include <ace/Process.h>

#include <utility>
#include <climits>

#include <QtGui>

#include "../../messaging/message_user.h"
#include "../../messaging/message_room.h"
#include "../../messaging/message_channel.h"
#include "../../messaging/message_send_room.h"
#include "../../messaging/message_room_command.h"
#include "../../os_util.h"
#include "../../config_file.h"
#include "../../model/house.h"
#include "../../model/self.h"
#include "../../executable/launcher.h"
#include "../../icon_store.h"
#include "../../rv_cmdline_builder.h"
#include "../../win_registry.h"
#include "../house_app.h"
#include "room_settings.h"
#include "room.h"
#include "house.h"

namespace gui {
namespace window {

room::room(const chat_gaming::room::id_type &id, QWidget *parent) :
        // WindowFlagst Qt::Dialog to make the room window stay on top of the parent (main window)
      size_restoring_window<QMainWindow>("room_win", parent, Qt::Dialog),
      _running_modal(false), _room_id(id)
{
    ACE_DEBUG((LM_DEBUG, "room::ctor, parent %d\n", parent));
    this->setWindowIcon(app_icons()->get("rv_house"));
    this->setContextMenuPolicy(Qt::NoContextMenu);

    _create_actions();
    _create_widgets();
    _create_toolbars();
    _create_layout();

    _connect_signals();

    _init();
}

void
room::_create_actions() {
    _action_quit     = new QAction(app_icons()->get("close"),    langstr("room_win/leave"),    this);
    _action_settings = new QAction(app_icons()->get("settings"), langstr("room_win/settings"), this);
    _action_launch   = new QAction(app_icons()->get("launch"),   langstr("room_win/launch"),   this);

}

void
room::_create_toolbars() {
    QToolBar *tool_bar = addToolBar("room_controls");
    tool_bar->setObjectName("room_toolbar_controls");
    tool_bar->addAction(_action_quit);
    tool_bar->addAction(_action_settings);
    tool_bar->addAction(_action_launch);

    tool_bar = addToolBar("room_infos");
    tool_bar->setObjectName("room_toolbar_infos");
    _info_container = new QWidget(tool_bar);

    tool_bar->addWidget(_info_container);
}

void
room::_create_widgets() {
    _chat_view  = new view::chat(this, _room_id);
    _users_view = new view::users(this, _room_id);
    _msg_field  = new QLineEdit(this);
    _msg_field->setMaxLength((int)app_opts.limit_chat_msg());

    _info_players = new QLabel(langstr("words/players"), this);
    _info_laps    = new QLabel(langstr("words/laps"),    this);
    _info_picks   = new QLabel(langstr("words/pickups"), this);

    // This ensures message field keeps the focus even if
    // user scrolls or otherwise operates the other widgets.
    // Is there a better way to do this?
    _chat_view->setFocusProxy(_msg_field);
    _users_view->setFocusProxy(_msg_field);
}

void
room::_connect_signals() {
    connect(_msg_field,       SIGNAL(returnPressed()), this, SLOT(send_message()));
    connect(_action_quit,     SIGNAL(triggered()),     this, SLOT(close()));
    connect(_action_settings, SIGNAL(triggered()),     this, SLOT(edit_room()));
    connect(_action_launch,   SIGNAL(triggered()),     this, SLOT(launch_rv()));
    connect(_users_view,      SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), house_win(), SLOT(open_private_room(QTreeWidgetItem *, int)));
}

void
room::_create_layout() {
    QVBoxLayout *l = new QVBoxLayout;
    QGridLayout *info_layout = new QGridLayout;

    // This is used in _create_toolbars

    _chat_users_splitter = new QSplitter(Qt::Horizontal);
    QSplitter *vsplitter = new QSplitter(Qt::Vertical);
    vsplitter->addWidget(_chat_view);
    _chat_users_splitter->addWidget(vsplitter);
    _chat_users_splitter->addWidget(_users_view);

    l->addWidget(_chat_users_splitter);
    l->addWidget(_msg_field);

    vsplitter->setStretchFactor(1, -1);
    _chat_users_splitter->setStretchFactor(0, -1);

    _users_view->observer_set(this);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(l);
    this->setCentralWidget(centralWidget);

    info_layout->setHorizontalSpacing(15);
    info_layout->setVerticalSpacing(0);
    info_layout->addWidget(new QLabel(langstr("words/players")), 0, 0);
    info_layout->addWidget(new QLabel(langstr("words/laps")),    1, 0);
    info_layout->addWidget(new QLabel(langstr("words/pickups")), 0, 2);
    info_layout->addWidget(_info_players, 0, 1);
    info_layout->addWidget(_info_laps,    1, 1);
    info_layout->addWidget(_info_picks,   0, 3);
    _info_container->setLayout(info_layout);

    _msg_field->setFocus();
}

void
room::show() {
    super::show();

    QList<int> _chat_users_splitter_sizes = _chat_users_splitter->sizes();
    _chat_users_splitter_sizes.back() = 150;
    _chat_users_splitter->setSizes(_chat_users_splitter_sizes);
}

void
room::_init() {
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

    if (_hosting &&
        pref()->get("general/send_ip", true) &&
        pref()->get("one_time_alert/send_ip", true))
    {
        pref()->set("one_time_alert/send_ip", false);
        const char *topic   = langstr("room_win/send_ip_info_title");
        const char *content = langstr("room_win/send_ip_info");

        variable_guard<bool> guard(_running_modal); _running_modal = true;

        QMessageBox msg_box(this);
        msg_box.setWindowTitle(topic);
        msg_box.setText(content);
        msg_box.setIcon(QMessageBox::Critical);
        msg_box.exec();
    }

    if (!_hosting) {
        // If not host, disable Edit and Launch buttons
        _action_settings->setDisabled(true);
        _action_launch->setDisabled(true);
    }

    _buttons_state();
}

room::~room() {
    ACE_DEBUG((LM_DEBUG, "room::dtor\n"));

    // Set ourself to be in no room anymore, and off we go
    self_model()->user().room_id(chat_gaming::room::id_type());
    self_model()->hosting_room().id(chat_gaming::room::id_type());
    self_model()->user_send();
    ACE_DEBUG((LM_DEBUG, "room: dtor done\n"));
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
    this->setWindowTitle(r->topic().c_str());

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

void
room::send_message() {
    QString t = _msg_field->text();

    if (t.isEmpty()) return;

    if (!_flood_control.allow_send()) {
        _chat_view->status_message(langstr("chat/flood_control"));
        return;
    }

    if (t.length() > (int)app_opts.limit_chat_msg())
        t.truncate(app_opts.limit_chat_msg());
    // IMPROVE Qt is this needed?
    // t.substitute("\r", "");

    // Sending a room message
    message_send_room *msg =
      new message_send_room(::message::send_room,
                            t.toLatin1().constData(),
                            self_model()->user().id(),
                            _room_id,
                            self_model()->sequence(),

                            0);
    net_messenger()->send_msg(msg);

    _msg_field->setText("");
}

void
room::edit_room() {
    ACE_DEBUG((LM_DEBUG, "room::edut:room()\n"));
    if (_room_id == self_model()->hosting_room().id()) {
        room_settings *win = new room_settings(this);
        win->exec();
    }
}

void
room::launch_rv() {
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
            int rejoin_time = conf()->get<int>("play/rejoin_time", INT_MAX);
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

                QMessageBox::information(this, topic, content.c_str());
            } else {
                _launch_join(playing_host);
            }
        }
    }
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

void
room::_launcher_error(int err) {
    const char *topic   = langstr("rv_launch/title_err");
    const char *content = "Could not launch";
    switch(err) {
    case executable::launcher::err_already_running:
        content = langstr("rv_launch/already_running");
        break;
    case executable::launcher::err_app_not_found:
        content = langstr("rv_launch/not_found");
        break;
    case executable::launcher::err_could_not_launch_rvgl:
        content = langstr("rv_launch/not_found_rvgl");
        break;
    case executable::launcher::err_could_not_launch:
    default:
        content = langstr("rv_launch/other");
    }

    variable_guard<bool> guard(_running_modal); _running_modal = true;
    QMessageBox::warning(this, topic, content);
}

void
room::handle_message(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "room::handle_message %d\n", msg->id()));

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
                os::flash_window(this->parentWidget());
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
        // IMPROVE Qt is this anymore a problem with Qt?
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

    QTimer::singleShot(5000, this, SLOT(launch_rv()));
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
    } else {
        ::app()->status_message(langstr("chat/host_kicks_you", ri->topic().c_str()));
    }

    if (ui_kicked->id().id_str() == self_model()->user().id().id_str()) {
        // Close room if been kicked out... TODO, as can be seen
        // this can't be done when running modal...
        // IMPROVE Qt is this anymore a problem with Qt
        if (!_running_modal) {
            this->close();
        }
    }
}

void
room::_launch_host() {
    if (conf()->get<bool>("play/no_launch", false) == false) {
        int ret;
        if (self_model()->room_version() && self_model()->room_version_rvgl()) {
            ret = launcher_rvgl()->start_host();
        } else {
            ret = launcher_game()->start_host();
        }
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
    if (conf()->get<bool>("play/no_launch", false) == false) {
        model::house::room_iterator ri = house_model()->room_find(_room_id);
        if (ri == house_model()->room_end()) {
            ACE_DEBUG((LM_ERROR, "room::_launch_join no room found "
            "for id %s\n", _room_id.c_str()));
            return;
        }

        int ret;
        if (ri->version_rvgl()) {
            if (self_model()->user().ip_as_string() == host_ui->ip_as_string()) {
                ret = launcher_rvgl()->start_client(std::string("0"));
            } else {
                ret = launcher_rvgl()->start_client(host_ui->ip_as_string());
            }
        } else {
            ret = launcher_game()->start_client(host_ui->ip_as_string());
        }
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
    QMessageBox::information(this, topic, content);

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
        _action_launch->setEnabled(true);
    } else {
        ACE_DEBUG((LM_DEBUG, "room::_buttons_state: disabling launch button\n"));
        _action_launch->setEnabled(false);
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
        && pref()->get("general/send_ip", true)
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
        !self_model()->user().getting_tracks()) {
        // Sharing tracks has been deprecated for now. Requires work for Qt if enabled again.
        // _share_button->enable();
    } else {
       // Sharing tracks has been deprecated for now. Requires work for Qt if enabled again.
       // _share_button->disable();
   }
}

} // ns window
} // ns gui
