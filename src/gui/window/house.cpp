#include <utility>
#include <iterator>

#include <fx.h>
#include <fxkeys.h>

#include "../../messaging/message_user.h"
#include "../../messaging/message_room.h"
#include "../../messaging/message_send.h"
#include "../../util.h"
#include "../../model/house.h"
#include "../../model/self.h"
#include "../../icon_store.h"
#include "../../app_version.h"

#include "../house_app.h"
#include "../util/util.h"
#include "about.h"
#include "room_settings.h"
#include "router_fw_help.h"
#include "house.h"

namespace gui {
namespace window {

FXDEFMAP(house) house_map[]= {
                                 FXMAPFUNCS(SEL_COMMAND, house::ID_CONNECT,
                                            house::ID_DISCONNECT,
                                            house::on_network_command),
                                 FXMAPFUNC(SEL_COMMAND,  house::ID_SEND_MSG,
                                           house::on_send_message),
                                 FXMAPFUNC(SEL_COMMAND,  house::ID_ROOM_CREATE,
                                           house::on_room_create),
                                 FXMAPFUNC(SEL_COMMAND,  house::ID_ROOM_JOIN,
                                           house::on_room_join),
                                 FXMAPFUNC(SEL_COMMAND,  house::ID_INTERRUPT,
                                           house::on_interrupt),
                                 FXMAPFUNC(SEL_COMMAND,  house::ID_REFRESH,
                                           house::on_refresh),
                                 FXMAPFUNC(SEL_TIMEOUT,  house::ID_REFRESH_ENABLE,
                                           house::on_refresh_enable),
                                 FXMAPFUNC(SEL_COMMAND,  house::ID_ABOUT,
                                           house::on_about),
                                 FXMAPFUNC(SEL_TIMEOUT,  house::ID_RECONNECT,
                                           house::on_reconnect),
                                 FXMAPFUNC(SEL_TIMEOUT,  house::ID_DHT_DISCONNECTED,
                                           house::on_dht_disconnected),
                             };

FXIMPLEMENT(house, FXMainWindow, house_map, ARRAYNUMBER(house_map))

#define STRING_JOIN(target, sep, src) \
target = (target.empty() ? src : (src.empty() ? target : target + sep + src))

// FXIMPLEMENT(house, FXMainWindow, NULL, 0)

house::house(FXApp *a)
        : FXMainWindow(a, APP_NAME, NULL, NULL, DECOR_ALL, 0, 0, 800, 600),
        _status_dht_extra(2),
        _last_connect(0),
        _flood_last_sent_message(0),
        _flood_control(0),
        _conn_tries(0),
        _ctz_disconnected(true),
        _last_dht_status_message_id(0)
{
    FXVerticalFrame *c = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    setIcon(app_icons()->get("rv_house"));
    setMiniIcon(app_icons()->get("rv_house"));

    _toolbar  = new FXHorizontalFrame(c);

    int button_opts = ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED;
    /* No connect/disconnect things for now
    new FXButton(_toolbar, "Connect",    NULL, this, ID_CONNECT);
    new FXButton(_toolbar, "Disconnect", NULL, this, ID_DISCONNECT);
    */
    // new FXButton(_toolbar, "Exit",       NULL, getApp(), FXApp::ID_QUIT);
    // new FXButton(_toolbar, "Exit", NULL, this, ID_CLOSE,
    //             button_opts);


    new FXButton(_toolbar,
                 util::button_text(NULL, langstr("main_win/exit")),
                 app_icons()->get("exit"), this, ID_CLOSE,
                 button_opts);

    new FXVerticalSeparator(_toolbar);

    _room_create_button =
        new FXButton(_toolbar,
                     util::button_text(NULL, langstr("main_win/create_room")),
                     app_icons()->get("room_create"),
                     this, ID_ROOM_CREATE, button_opts);
    _room_join_button =
        new FXButton(_toolbar,
                     util::button_text(NULL, langstr("main_win/join_room")),
                     app_icons()->get("room_join"),
                     this, ID_ROOM_JOIN, button_opts);
    _refresh_button =
        new FXButton(_toolbar,
                     util::button_text(NULL, langstr("main_win/refresh")),
                     app_icons()->get("refresh"),
                     this, ID_REFRESH, button_opts);

    _interrupt_button =
        new FXButton(_toolbar,
                     util::button_text(NULL, langstr("main_win/cancel")),
                     app_icons()->get("cancel"),
                     this, ID_INTERRUPT, button_opts);

    new FXVerticalSeparator(_toolbar);

    new FXButton(_toolbar,
                 util::button_text(NULL, langstr("main_win/about")),
                 app_icons()->get("about"),
                 this, ID_ABOUT, button_opts);

    FXSplitter *house     = new FXSplitter(c, SPLITTER_HORIZONTAL |
                                           SPLITTER_REVERSED   |
                                           LAYOUT_FILL_X |
                                           LAYOUT_FILL_Y);
    FXSplitter *sections  = new FXSplitter(house, LAYOUT_FILL_X |
                                           LAYOUT_FILL_Y |
                                           SPLITTER_VERTICAL);
    _users_view = new view::users(house, NULL); // , 0, FRAME_SUNKEN|FRAME_THICK);
    new FXFrame(_users_view);
    // new FXLabel(house, "Users View", NULL, FRAME_SUNKEN|FRAME_THICK);
    _rooms_view = new view::rooms(sections);
    _rooms_view->target_item_doubleclicked(this, FXSEL(SEL_COMMAND, ID_ROOM_JOIN));

    // new FXLabel(sections, "Rooms View", NULL, FRAME_SUNKEN|FRAME_THICK);
    _chat_view = new view::chat(sections);
    // new FXLabel(sections, "Chat Window View", NULL, FRAME_SUNKEN|FRAME_THICK);
    _msg_field = new FXTextField(c, 0,  this, ID_SEND_MSG,
                                 FRAME_SUNKEN|FRAME_THICK|
                                 LAYOUT_FILL_X|TEXTFIELD_ENTER_ONLY);
    _msg_field->setFocus();

    _status   = new FXStatusBar(c, LAYOUT_FILL_X);

    _update_status();
    // _status->getStatusLine()->setNormalText("Waiting...");

    _rooms_view->setHeight(150);
    _users_view->setWidth(150);
    _rooms_view->observer_set(this);
    _users_view->observer_set(this);

    getAccelTable()->addAccel(MKUINT(KEY_F4,ALTMASK),this,FXSEL(SEL_COMMAND,ID_CLOSE));
}

void
house::create() {
    FXMainWindow::create();
    watched_window::create(this);
    interruptable_action_update();

    _room_buttons_status();
    // TODO away
    on_dht_disconnected(this, 0, 0);
}

house::~house() {
    getApp()->removeTimeout(this, ID_RECONNECT);
    getApp()->removeTimeout(this, ID_REFRESH);
    getApp()->removeTimeout(this, ID_DHT_DISCONNECTED);

    ACE_DEBUG((LM_DEBUG, "Main window closed, sending quit request to APP\n"));

    getApp()->handle(this, FXSEL(SEL_COMMAND,FXApp::ID_QUIT), NULL);
}

long
house::on_send_message(FXObject *from, FXSelector sel, void *) {
    FXString t = _msg_field->getText();

    if (t.empty()) return 1;

    // Simple flood control
    ACE_Time_Value now(ACE_OS::gettimeofday());

    if (_flood_control < 0 || (now - _flood_last_sent_message).msec() >= 5000)
        _flood_control = 0;
    else
        _flood_control += 2000 - (now - _flood_last_sent_message).msec();

    if (_flood_control >= 3000) {
        _chat_view->status_message(langstr("chat/flood_control"));
        return 0;
    }
    // if (now - _last_send_message < 2) return 0;

    if (t.length() > (int)app_opts.limit_chat_msg())
        t.trunc(app_opts.limit_chat_msg());
    t.substitute("\r", "");

    // Check for a command
    if (!_chat_command(t)) {
        // Sending a public message
        message_send *msg =
            new message_send(::message::send,
                             t.text(),
                             self_model()->user().id(),
                             self_model()->sequence(),
                             0);
        net_messenger()->send_msg(msg);
    }

    _msg_field->setText("");
    _flood_last_sent_message = now;

    return 1;
}

bool
house::_chat_command(const FXString &t) {
    if (t[0] == '/') {
        std::string cmd = t.text() + 1;
        if (cmd == "model_rooms") {
            _chat_command_model_rooms();
        } else if (cmd == "model_users") {
            _chat_command_model_users();
        } else if (cmd == "model_self") {
            _chat_command_model_self();
        } else {
            _chat_view->status_message("Invalid command: " + cmd);
        }
        return true;
    }
    return false;
}

void
house::_chat_command_model_rooms() {
    // TODO refactor into a common class so that functionality is available
    // from every chat_view.
    std::string indent;
    char        indent_char = ' ';
    size_t      indent_size = 0;

    const
    model::house::group_desc_type &g = house_model()->group_desc();
    model::house::group_desc_type::const_iterator gi = g.begin();
    for (; gi != g.end(); gi++) {
        _chat_view->status_message(indent + "Group " + gi->second + " {");
        indent.resize(indent_size += 2, indent_char);
        {
            chat_gaming::house &h = house_model()->house_find(gi->first);
            chat_gaming::house::room_iterator ri = h.room_begin();
            for (; ri != h.room_end(); ri++) {
                _chat_view->status_message(indent + "Room " + ri->id() + " {");
                indent.resize(indent_size += 2, indent_char);
                {
                    _chat_view->status_message(indent + "Topic: " + ri->topic());
                    _chat_view->status_message(indent + "Owner ID: " + ri->owner_id().id_str());
                    _chat_view->status_message(indent + "Players " + " {");
                    indent.resize(indent_size += 2, indent_char);
                    {
                        typedef std::list<chat_gaming::user::id_type> ltype;
                        ltype userids;
                        std::insert_iterator<ltype> ins_itr(userids,userids.end());
                        house_model()->user_ids_in_room(ri->id(), ins_itr, gi->first);

                        for (ltype::iterator i = userids.begin();
                                i != userids.end(); i++)
                            _chat_view->status_message(indent + i->c_str());
                    }
                    indent.resize(indent_size -= 2, indent_char);
                    _chat_view->status_message(indent + "}");
                }
                indent.resize(indent_size -= 2, indent_char);
                _chat_view->status_message(indent + "}");
            }
        }
        indent.resize(indent_size -= 2, indent_char);
        _chat_view->status_message(indent + "}");

    }
}

void
house::_chat_command_model_users() {
    // TODO refactor into a common class so that functionality is available
    // from every chat_view.
    std::string indent;
    char        indent_char = ' ';
    size_t      indent_size = 0;

    const
    model::house::group_desc_type &g = house_model()->group_desc();
    model::house::group_desc_type::const_iterator gi = g.begin();
    for (; gi != g.end(); gi++) {
        _chat_view->status_message(indent + "Group " + gi->second + " {");
        indent.resize(indent_size += 2, indent_char);
        {
            chat_gaming::house &h = house_model()->house_find(gi->first);
            chat_gaming::house::user_iterator ui = h.user_begin();
            for (; ui != h.user_end(); ui++) {
                const netcomgrp::node *node = ui->node();

                _chat_view->status_message(indent + "User " + (std::string)(ui->id()) + " {");
                indent.resize(indent_size += 2, indent_char);
                {
                    _chat_view->status_message(indent + "Login ID  : " + ui->login_id());
                    _chat_view->status_message(indent + "Display ID: " + ui->display_id());
                    _chat_view->status_message(indent + "Validation: " + ui->validation());
                    _chat_view->status_message(indent + "Room ID: "    + ui->room_id());
                    _chat_view->status_message(
                        indent + "IP: " +
                        (node
                         ? multi_feed::addr_to_string()(node->addr(), gi->first)
                         : ui->ip_as_string())
                    );
                }
                indent.resize(indent_size -= 2, indent_char);
                _chat_view->status_message(indent + "}");
            }
        }
        indent.resize(indent_size -= 2, indent_char);
        _chat_view->status_message(indent + "}");
    }
}

void
house::_chat_command_model_self() {
    // TODO refactor into a common class so that functionality is available
    // from every chat_view.
    std::string indent;
    char        indent_char = ' ';
    size_t      indent_size = 0;

    _chat_view->status_message(indent + "Self {");
    indent.resize(indent_size += 2, indent_char);
    {
        const chat_gaming::user &u = self_model()->user();
        const chat_gaming::room &r = self_model()->hosting_room();
        std::ostringstream seq_stream;
        seq_stream << self_model()->sequence();

        _chat_view->status_message(indent + "ID: " + u.id().c_str());
        _chat_view->status_message(indent + "Display ID: " + u.display_id());
        _chat_view->status_message(indent + "Login ID: " + u.login_id());
        _chat_view->status_message(indent + "Validation: " + u.validation());
        _chat_view->status_message(indent + "IP: " + u.ip_as_string());
        _chat_view->status_message(indent + "Sequence: " + seq_stream.str());


        if (u.room_id() != chat_gaming::room::id_type()) {
            _chat_view->status_message(indent + "In Room ID: " + u.room_id());
        }
        if (self_model()->joining_room() != chat_gaming::room::id_type()) {
            _chat_view->status_message(indent + "Joining room " + self_model()->joining_room());
        }
        if (r.id() != chat_gaming::room::id_type()) {
            _chat_view->status_message(indent + "Hosting Room " + r.id() + " {");
            indent.resize(indent_size += 2, indent_char);
            {
                _chat_view->status_message(indent + "Topic: " + r.topic());
            }
            indent.resize(indent_size -= 2, indent_char);
            _chat_view->status_message(indent + "}");
        }
    }
    indent.resize(indent_size -= 2, indent_char);
    _chat_view->status_message(indent + "}");
}

long
house::on_network_command(FXObject *from, FXSelector sel, void *) {
    ::message *msg = NULL;

    switch (FXSELID(sel)) {
    case ID_CONNECT:
        {
            chat_gaming::user u(self_model()->user());
            // u.login_id(self_model()->user());
            // if (self_model()->user_validated())
            //	u.validation("1");

            ACE_DEBUG((LM_DEBUG, "TODO debug user id str: %s\n",
                       self_model()->user().id().id_str().c_str()));
            msg = new message_user(::message::connect,
                                   self_model()->user(),
                                   self_model()->sequence(), 0);
            _conn_tries++;
            _last_connect = time(NULL);
        }
        break;
    case ID_DISCONNECT:
        msg = new ::message(::message::disconnect);
        break;
    default:
        ACE_DEBUG((LM_DEBUG, "house::on_network_command: unrecognized " \
                   " message id %d\n", FXSELID(sel)));
    }

    if (msg) net_messenger()->send_msg(msg);

    return 1;
}

long
house::on_room_create(FXObject *from, FXSelector sel, void *) {
    ACE_DEBUG((LM_DEBUG, "house: creating room\n"));

    // TODO Don't allow creating a new room if the user is already in another
    // room.
    window::room_settings *win = new window::room_settings(this);

    if (!win->execute(PLACEMENT_SCREEN)) {
        ACE_DEBUG((LM_DEBUG, "house::Room create cancelled\n"));
        return 1;
        // handle(this, FXSEL(SEL_COMMAND, ID_QUIT), NULL);
    }

    ACE_DEBUG((LM_DEBUG, "house::Room create successfull\n"));

    return 1;
}

long
house::on_room_join(FXObject *from, FXSelector sel, void *ptr) {
    ACE_DEBUG((LM_DEBUG, "house: joining room\n"));

    if (!self_model()->user().room_id().empty()) {
        ACE_DEBUG((LM_DEBUG, "house: already in room id %s, ignored\n",
                   self_model()->user().id().c_str()));
        return 0;
    }
    if (!self_model()->joining_room().empty()) {
        ACE_DEBUG((LM_DEBUG, "house: already joining room id %s, ignored\n",
                   self_model()->joining_room().c_str()));
        return 0;
    }

    int i = _rooms_view->selected_item_index();
    ACE_DEBUG((LM_DEBUG, "house::index clicked is %d\n", i));

    if (i < 0) return 0;

    // room_item *item = _rooms_view->item_at(i);
    chat_gaming::room::id_type rid = _rooms_view->room_id_at(i);

    if (rid.empty()) {
        ACE_DEBUG((LM_ERROR, "house::on_room_join no matching room "
                   "found for index %d\n", i));
        return 0;
    }

    model::house::house_type::room_iterator ri = house_model()->room_find(rid);
    if (ri == house_model()->room_end()) {
        ACE_DEBUG((LM_ERROR, "house::on_room_join no matching room "
                   "found from model for room id %s\n", rid.c_str()));
        return 0;
    }

    chat_gaming::room r(*ri);
    // Check if the room requires a password and ask for it if so
    if (r.has_password()) {
        FXInputDialog passinp(this,
                              langstr("main_win/ask_room_pass_title"),
                              langstr("main_win/ask_room_pass"));
        if (!passinp.execute()) {
            // Join cancelled when asked for password
            return 1;
        }
        r.password(passinp.getText().text());
    }
    // TODO message_room_join would be more appropriate, change when time
    // permits, this one will do for now
    message_room *m = new message_room(
                          message::room_join,
                          r,
                          self_model()->user().id(),
                          self_model()->sequence(),
                          0
                      );
    net_messenger()->send_msg(m);
    self_model()->joining_room(r.id());
    ::app()->interruptable_action_update(langstr("main_win/joining_room"));
    // return 0;

    return 1;
}

void
house::handle_message(::message *msg) {
    if (handle_dht_message(msg)         ||
            handle_ctz_message(msg)         ||
            handle_external_ip_message(msg))
    {
        _update_status();
        return;
    }

    switch(msg->id()) {
    case ::message::room:
    case ::message::user:
        _room_buttons_status(); break;
    }
    _users_view->handle_message(msg);
    _chat_view->handle_message(msg);
    _rooms_view->handle_message(msg);
}



bool
house::handle_dht_message(::message *msg) {
    switch (msg->id()) {
    case message::dht_connecting:      _status_dht = langstr("main_win/dht_connecting");    break;
    case message::dht_connected:       _status_dht = langstr("main_win/dht_connected");     break;
    case message::dht_disconnecting:   _status_dht = langstr("main_win/dht_disconnecting"); break;
    case message::dht_disconnected:    _status_dht = langstr("main_win/dht_disconnected");     break;
    case message::dht_upd_nodes:       _status_dht = langstr("main_win/dht_upding_contacts"); break;
    case message::dht_upd_nodes_done:  _status_dht = langstr("main_win/dht_upd_contacts"); break;
    case message::dht_upd_nodes_fail:  _status_dht = langstr("main_win/dht_upd_contacts_fail"); break;
    case message::dht_group_joining:   _status_dht = langstr("main_win/dht_joining"); break;
    case message::dht_group_joined:    _status_dht = langstr("main_win/dht_joined"); break;
    case message::dht_group_leaving:   _status_dht = langstr("main_win/dht_leaving"); break;
    case message::dht_group_not_joined:_status_dht = langstr("main_win/dht_left"); break;
    case message::dht_peers_find_start:_status_dht_extra[0] = langstr("main_win/dht_searching"); break;
    case message::dht_peers_find_stop: _status_dht_extra[0] = ""; break;
    case message::dht_announce_start:  _status_dht_extra[1] = langstr("main_win/dht_announcing"); break;
    case message::dht_announce_stop:   _status_dht_extra[1] = ""; break;
    default:
        return false;
    }

    switch (msg->id()) {
    case message::dht_connecting:
    case message::dht_connected:
    case message::dht_disconnecting:
    case message::dht_disconnected:
        _last_dht_status_message_id = msg->id();
    }

    if (msg->id() == message::dht_disconnected) {
        ACE_DEBUG((LM_DEBUG, "house: adding a DHT disconnect timer\n"));
        // Install a timer that checks if still in dht_disconnected
        // mode, and if so it displays a help. A timer is needed
        // because even if we receive dht_disconnected, the networking
        // thread might decide to retry right after. By starting a timer
        // and checking if the state is still dht_disconnected we
        // get a reliable result.
        getApp()->addTimeout(this, ID_DHT_DISCONNECTED, 5000, NULL);
    }
#if 0
    if (msg->id() == message::dht_group_joined) {
        // Must send updated status once joined in case the user has been
        // doing something before full connect status reached.
        self_model()->state_send();
    }
#endif

    return true;
}

bool
house::handle_ctz_message(::message *msg) {
    switch (msg->id()) {
    case message::ctz_group_joining:   _status_ctz = langstr("main_win/ctz_connecting"); break;
    case message::ctz_group_joined:
        _status_ctz   = langstr("main_win/ctz_connected");
        _conn_tries = 0;
        break;
    case message::ctz_group_leaving:   _status_ctz = langstr("main_win/ctz_disconnecting"); break;
    case message::ctz_group_not_joined:
        {
            _ctz_disconnected = true;
            _status_ctz = langstr("main_win/ctz_disconnected");
            // Install a timer hook that causes a connection retry.
            // The more times reconnect has been tried without getting
            // a connection, the longer time to wait. Give up
            // altogether if more than 5 times tried.
            if (_conn_tries < 5) {
                int conn_in_secs = 30 * _conn_tries;
                ACE_DEBUG((LM_DEBUG, "house::installing reconnect timer %d secs\n",
                           conn_in_secs));
                getApp()->addTimeout(this, ID_RECONNECT, conn_in_secs*1000, NULL);
            }
        }
        break;
    case message::ctz_group_server_unreachable:
        {
            // TODO, maybe a dialog if both ctz and dht can not be reached.
            /*		message_string *m = dynamic_ptr_cast<message_string>(msg);
            		std::string cnt = "Connection timeout to centralized server: " + 
            		                  m->str();
            		FXMessageBox::error(this, FX::MBOX_OK, "Server unreachable",
            	    	                cnt.c_str());
            */
        }
        break;
    default:
        return false;
    }

    _ctz_disconnected = (msg->id() == message::ctz_group_not_joined
                         ? true : false);

    return true;
}

bool
house::handle_external_ip_message(::message *msg) {
    message_string *ms = dynamic_cast<message_string *>(msg);
    if (!ms) return false;

    switch (ms->id()) {
    case message::external_ip_fetching:
        _status_tmp = langstr("main_win/ip_detecting", ms->str().c_str()); break;
    case message::external_ip_fetch_done:
        _status_tmp = langstr("main_win/ip_detected",  ms->str().c_str()); break;
    case message::external_ip_fetch_fail:
        _status_tmp = langstr("main_win/ip_failed",    ms->str().c_str()); break;
    default:
        return false;
    }

    _update_status();
    return true;
}

void
house::_update_status() {
    std::string status;

    STRING_JOIN(status, "; ", _status_ctz);
    STRING_JOIN(status, "; ", _status_dht);
    STRING_JOIN(status, "; ", _status_dht_extra[0]);
    STRING_JOIN(status, "; ", _status_dht_extra[1]);
    STRING_JOIN(status, "; ", _status_tmp);

    _status->getStatusLine()->setNormalText(status.c_str());
    // _status->setText(status.c_str());
}

void
house::_room_buttons_status() {
    if (self_model()->joining_room()   != chat_gaming::room::id_type() ||
            self_model()->user().room_id() != chat_gaming::room::id_type())
    {
        _room_create_button->disable();
        _room_join_button->disable();
    } else {
        _room_create_button->enable();
        _room_join_button->enable();
    }
}

long
house::on_interrupt(FXObject *from, FXSelector sel, void *) {
    if (!self_model()->joining_room().empty()) {
        self_model()->joining_room(chat_gaming::room::id_type());
        // If aborted before joined, necessary to do this update so that
        // the recipient won't reserve a slot for the joiner anymore.
        self_model()->user_send();
    }

    interruptable_action_update();
    return 1;
}

long
house::on_refresh  (FXObject *from, FXSelector sel, void *) {
    ::message *m = new ::message(::message::refresh);
    net_messenger()->send_msg(m);

    // disable refresh button for 8 seconds
    getApp()->addTimeout(this, ID_REFRESH_ENABLE, 8000, NULL);
    _refresh_button->disable();
    return 1;
}

long
house::on_refresh_enable  (FXObject *from, FXSelector sel, void *) {
    _refresh_button->enable();
    return 1;
}

long
house::on_reconnect (FXObject *from, FXSelector sel, void *) {
    ACE_DEBUG((LM_DEBUG, "house::on_reconnect received\n"));
    if (_ctz_disconnected) {
        ACE_DEBUG((LM_DEBUG, "house::trying to reconnect\n"));
        // If still disconnected, make a retry connecting
        this->handle(this, FXSEL(SEL_COMMAND, ID_CONNECT), NULL);
    }

    return 1;
}

long
house::on_about (FXObject *from, FXSelector sel, void *) {
    gui::window::about *win  = new gui::window::about(this);

    win->execute(PLACEMENT_SCREEN);

    return 1;
}

void
house::interruptable_action_update() {
    // True if interruptable
    bool intr = false;
    if (!self_model()->joining_room().empty()) {
        intr = true;
    }

    intr ? _interrupt_button->enable() : _interrupt_button->disable();
    _status_tmp.clear();
    _update_status();
}

long
house::on_dht_disconnected(FXObject *from, FXSelector sel, void *) {
    ACE_DEBUG((LM_DEBUG, "house::on_dht_disconnected\n"));
    if (_last_dht_status_message_id != message::dht_disconnected) {
        ACE_DEBUG((LM_DEBUG, "house: was not disconnected anymore, "
                   "%d/%d\n",
                   _last_dht_status_message_id, message::dht_disconnected));
        /* No more on disconnected state, either retrying or succeeded,
         * so do nothing unless receive disconnected again
         */
        return 1;
    }

    ACE_DEBUG((LM_DEBUG, "house::on_dht_disconnected:help box\n"));
    // Display a message box
    (new router_fw_help)->show_when_possible();

    return 1;
}

void
house::interruptable_action_update(const std::string &s) {
    interruptable_action_update();

    _status_tmp = s;
    _update_status();
}

// users_view::observer interface
void
house::user_added(const chat_gaming::user &u) {
    _chat_view->status_message(langstr("chat/user_entered", u.display_id().c_str()));
}

void
house::user_removed(const chat_gaming::user &u) {
    _chat_view->status_message(langstr("chat/user_exited", u.display_id().c_str()));
}

// rooms_view::observer interface
void
house::room_added(const chat_gaming::room &r) {
    _chat_view->status_message(langstr("chat/room_created", r.topic().c_str()));
}

void
house::room_removed(const chat_gaming::room &r) {
    _chat_view->status_message(langstr("chat/room_closed", r.topic().c_str()));
}

} // ns window
} // ns gui
