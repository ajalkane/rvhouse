#include <sstream>
#include <iostream> 
#include <string>
#include <typeinfo>

#include <limits.h>

#include <ace/ACE.h>
#include <ace/Log_Msg.h>

#include <QtGui>

#include "house_app.h"

#include "../app_options.h"
#include "../config_file.h"
#include "../exception.h"
#include "../file_util.h"
#include "../main.h"
#include "../icon_store.h"
#include "../os_util.h"
#include "../win_registry.h"
#include "../executable/launcher.h"
#include "../executable/factory.h"
#include "../lang/manager.h"
#include "../messaging/message_string.h"
#include "../messaging/message_channel.h"
#include "../messaging/message_user.h"
#include "../messaging/message_room.h"
#include "../messaging/message_room_join.h"
#include "../messaging/message_grouped.h"
#include "../messaging/message_version.h"
#include "../messaging/message_global_ignore.h"
#include "../messaging/message_block_users.h"
#include "../messaging/impl/qt_messenger.h"
#include "../model/house.h"
#include "../model/self.h"
#include "message_handler.h"
#include "window/login.h"
#include "window/version_update.h"

namespace gui {

house_app::house_app(int &argc, char **argv)
    : QApplication(argc, argv),
      _house_win(NULL),
      _room_win(NULL)
{
    ACE_DEBUG((LM_DEBUG, "house_app: ctor, argc %d argv %d\n", argc, argv));
    for (int i = 0; i < argc; i++) {
        ACE_DEBUG((LM_DEBUG, "house_app: argv[%d]: '%s'\n", i, argv[i]));
    }

    ACE_DEBUG((LM_DEBUG, "house_app: ctor\n", argv[0]));

    _net_worker = new networking::worker;

    gui_messenger.instance(new qt_messenger(this, this));

    lang_mngr.instance(new lang::manager);

    house_model.instance(new model::house);
    house_model()->allow_group("DHT",         message::dht_group_base);
    house_model()->allow_group("centralized", message::ctz_group_base);
    house_model_prev.instance(new model::house(*(house_model())));

    self_model.instance(new model::self);
    launcher_game.instance(executable::factory().create_launcher("Re-Volt"));
    launcher_rvgl.instance(executable::factory().create_launcher("RVGL"));
    launcher_file.instance(executable::factory().create_launcher("File"));
    launcher_rvtm.instance(executable::factory().create_launcher("RVTM"));

    game_registry.instance(
        new win_registry(win_registry::id_software, "Acclaim", "Re-Volt", "1.0")
    );

    pref.instance(new config_file(config_file::registry));

    app_opts.init();

    app_icons.instance(new icon_store(this));
    app_icons()->scan_from_dir(
        app_rel_path("img"),
        "about", "cancel", "close", "exit", "launch", "launch_small",
        "locked_small", "refresh", "room_create", "room_join",
        "settings", "rv_house",
        "user_in_room_small", "user_away", "user_dont_disturb",
        "tracks_share", "tracks_share_small",
        "private_message",
        NULL
    );

    _init_language();

    ACE_DEBUG((LM_DEBUG, "house_app: creating main window\n"));

    this->setWindowIcon(app_icons()->get("rv_house"));
}

house_app::~house_app() {
    ACE_DEBUG((LM_DEBUG, "house_app: dtor\n"));
    delete _house_win;

    if (net_messenger()) {
        net_messenger()->send_msg(new ::message(::message::disconnect));
        net_messenger()->send_msg(new ::message(::message::exit_now));
        ACE_DEBUG((LM_DEBUG, "Waiting for networking thread to exit\n"));
        _net_worker->wait();
        ACE_DEBUG((LM_DEBUG, "Networking thread exited\n"));
        delete _net_worker;
    }

    delete gui_messenger();    gui_messenger.instance(NULL);
    delete lang_mngr();        lang_mngr.instance(NULL);
    delete house_model();      house_model.instance(NULL);
    delete house_model_prev(); house_model_prev.instance(NULL);
    delete self_model();    self_model.instance(NULL);
    delete launcher_game(); launcher_game.instance(NULL);
    delete launcher_rvgl(); launcher_rvgl.instance(NULL);
    delete launcher_file(); launcher_file.instance(NULL);
    delete launcher_rvtm(); launcher_rvtm.instance(NULL);
    delete game_registry(); game_registry.instance(NULL);
    delete app_icons();  app_icons.instance(NULL);
    delete lang_mngr();     lang_mngr.instance(NULL);
    delete pref();          pref.instance(NULL);
}

void
house_app::_init_language() {
    std::string sel_lang = conf()->get_value("main/lang");

    lang_mngr()->init(app_rel_path("lang"));
    // The language might have been removed, in that case force a reselect
    if (!lang_mngr()->language_exists(sel_lang)) {
        sel_lang.clear();
    }

    if (sel_lang.empty()) {
        const std::list<lang::info> inf = lang_mngr()->lang_infos();
        if (inf.size() == 1) {
            // If only one language, no use to show the selection dialog
            sel_lang = inf.front().lang();
        } else {
            QStringList choices;
            int default_index = 0;
            int index = 0;
            for (std::list<lang::info>::const_iterator i = inf.begin();
                 i != inf.end(); ++i, ++index)
            {
                std::string lang = i->lang();
                if (lang == "English") {
                    default_index = index;
                }
                choices.append(i->lang().c_str());
            }

            bool ok = false;
            QString lang = QInputDialog::getItem(NULL, "Language selection", "Choose language", choices, default_index, false, &ok);
            if (ok && !lang.isEmpty()) {
                sel_lang = lang.toLatin1().constData();
                conf()->set("main", "lang", sel_lang);
                conf()->save();
            }
        }
    }

    if (sel_lang.empty()) sel_lang = DEFAULT_LANGUAGE;

    lang_mngr()->mapper_init(lang_mapper, sel_lang, DEFAULT_LANGUAGE);
}

bool
house_app::start() {
    ACE_DEBUG((LM_DEBUG, "house_app: arguments size before new window::house %d\n", this->arguments().size()));
    if (!conf()->get<bool>("main", "no_instance_check", false)) {
        if (!os::ensure_single_app_instance()) {
            throw exception(0, langstr("app/rv_house_running"));
        }
    }
    ACE_DEBUG((LM_DEBUG, "start network worker\n"));
    _net_worker->activate();
    
    _house_win  = new window::house;

    _house_win->show();

#ifdef WIN32
    if (!game_registry()->is_open()) {
        QMessageBox::warning(_house_win, langstr("app/rv_not_found_title"), langstr("app/rv_not_found"));
    }
#endif

    ACE_DEBUG((LM_DEBUG, "house_app::start main/gui_test: %d\n",
              conf()->get<bool>("main/gui_test", false)));

    if (conf()->get<bool>("main/gui_test", false)) {
        ACE_DEBUG((LM_DEBUG, "house_app::start gui_test activated, no login shown\n"));
        return true;
    }

    // Fetch current version information
    net_messenger()->send_msg(new ::message(::message::version_fetch));
    // Fetch global ignore list unless feature disabled
    if (pref()->get<bool>("general/global_ignore", true)) {
        net_messenger()->send_msg(new ::message(::message::global_ignore_fetch));
    }

    ACE_DEBUG((LM_DEBUG, "house_app::start creating login window\n"));
    window::login *_login_win = new window::login(_house_win);
    std::auto_ptr<window::login> login_win_guard(_login_win);

    _login_win->user(pref()->get("user/id", std::string()));
    // Use rot13 in nostalgic honour of this legendary scrambler!
    _login_win->pass(derot13(pref()->get("user/pass", std::string())));
    ACE_DEBUG((LM_DEBUG, "login_window: executing %d\n", _login_win));
    int ret = _login_win->exec();
    ACE_DEBUG((LM_DEBUG, "login_window: return value %d\n", ret));
    if (ret == QDialog::Rejected) {
        ACE_DEBUG((LM_DEBUG, "Login failed, quitting\n"));
        quit();
        return false;
    }

    pref()->set("user/id", _login_win->user());
    // Use rot13 in nostalgic honour of this legendary scrambler!
    pref()->set("user/pass", rot13(_login_win->pass()));
    pref()->save();

    self_model()->user().login_id(_login_win->user());
    if (_login_win->user_validated())
        self_model()->user().validation("1");

    self_model()->user().status(chat_gaming::user::status_chatting);
    self_model()->user().generate_id();
    self_model()->state_send();

    if (_login_win->auto_connect()) {
        ACE_DEBUG((LM_DEBUG, "house_app::auto connect requested, "
                             "sending connect\n"));
        _house_win->net_connect();
    }

    return true;
}

bool
house_app::event(QEvent *event) {
    if (event->type() == QEvent::User) {
        _handle_messenger();
        return true;
    }
    return QApplication::event(event);
}

bool
house_app::_handle_messenger() {
    ACE_DEBUG((LM_DEBUG, "house_app: handle_messenger received at thread %t\n"));

    typedef std::list<message *> ctype;
    ctype msgs;
    std::insert_iterator<ctype> inserter(msgs, msgs.begin());

    gui_messenger()->collect_msgs(inserter);

    ACE_DEBUG((LM_DEBUG, "house_app: %d messages to process\n", msgs.size()));
    try {
        ctype::iterator i = msgs.begin();
        for (; i != msgs.end(); i++) {
            ACE_DEBUG((LM_DEBUG, "house_app: msg is %d\n", (*i)->id()));

            if (_filter_message(*i)) continue;

            ACE_DEBUG((LM_DEBUG, "house_app: handle_model_update\n"));
            _handle_model_update(house_model(), *i);

            // This has to be before the switch, because room_window is
            // responsible for closing itself, which must be done before
            // contemplating on the possibility of opening another room_window
            ACE_DEBUG((LM_DEBUG, "house_app: iterating\n"));
            for (_watched_list_type::iterator wi = _watched_list.begin();
                 wi != _watched_list.end();)
            {
                message_handler *h = dynamic_cast<message_handler *>(*wi);
                // Have to increase the iterator before handling the message
                // since the message handling object might delete itself due
                // to the message, thus invalidating the iterator
                wi++;
                if (h) {
                    ACE_DEBUG((LM_DEBUG, "house_app: sending message to handler\n"));
                    h->handle_message(*i);
                }
            }

            ACE_DEBUG((LM_DEBUG, "house_app: rest of the shit\n"));

            switch ((*i)->id()) {
            case ::message::worker_exception:
                _handle_worker_exception(*i);
                break;
            case ::message::room:
            case ::message::user:
                _cond_open_room_window(*i);
                break;
            case ::message::send_private:
                _cond_open_private_message_window(*i);
                break;
            case ::message::room_join:
                _handle_room_join(*i);
                break;
            case ::message::room_join_full:
            case ::message::room_join_pass:
            case ::message::room_join_nack:
            case ::message::room_join_ack:
                _handle_room_join_ack(*i);
                break;
            case ::message::version:
            case ::message::version_force:
                _version_update_window(*i);
                break;
            case ::message::global_ignore_list:
                _handle_global_ignore(*i);
                break;
            case ::message::rvtm_exited:
                ACE_DEBUG((LM_DEBUG, "house_app::rvtm_exited msg received\n"));
                self_model()->user().sharing_tracks(false);
                self_model()->user().getting_tracks(false);
                self_model()->user_send();
                break;
            }

            ACE_DEBUG((LM_DEBUG, "house_app: prev update\n"));

            _handle_model_update(house_model_prev(), *i);
            ACE_DEBUG((LM_DEBUG, "house_app: out\n"));

        }
    } catch (...) {
        // Ensure all messages are deleted even in case of an error
        ACE_DEBUG((LM_DEBUG, "deleting all messages\n"));
        std::for_each(msgs.begin(), msgs.end(), delete_ptr<message>());
        ACE_DEBUG((LM_DEBUG, "throwing exception forward\n"));
        throw;
    }
    ACE_DEBUG((LM_DEBUG, "deleting all messages\n"));
    std::for_each(msgs.begin(), msgs.end(), delete_ptr<message>());

    ACE_DEBUG((LM_DEBUG, "house_app: handle_messenger exiting\n"));

    return true;
}

void
house_app::interruptable_action_update() {
    if (_house_win) _house_win->interruptable_action_update();
}

void
house_app::interruptable_action_update(const std::string &s) {
    if (_house_win) _house_win->interruptable_action_update(s);
}

void
house_app::watched_object_created(QObject *obj) {
    ACE_DEBUG((LM_DEBUG, "house_app::on_object_create received (list size %d)\n",
              _watched_list.size()));

    _watched_list.push_back(obj);

    ACE_DEBUG((LM_DEBUG, "house_app::on_object_create list size after %d\n",
              _watched_list.size()));
}

void
house_app::watched_object_destroyed(QObject *obj) {
    ACE_DEBUG((LM_DEBUG, "house_app::on_object_destroy received (list size %d)\n",
              _watched_list.size()));

    _watched_list.remove(obj);

    ACE_DEBUG((LM_DEBUG, "house_app::on_object_destroy list size after %d\n",
              _watched_list.size()));

    // IMPROVE: generic interface for windows that are auto deleted on close.
    if (static_cast<window::room *>(obj) == _room_win) {
        ACE_DEBUG((LM_DEBUG, "house_app::on_object_destroy, was room_window %d\n", _room_win));
        _room_win = NULL;
    }
    if (static_cast<window::house *>(obj) == _house_win) {
        ACE_DEBUG((LM_DEBUG, "house_app::on_object_destroy, was house_window\n"));
        _house_win = NULL;
    }
}

void
house_app::_handle_room_join(::message *msg) {
    message_room_join *m = dynamic_ptr_cast<message_room_join>(msg);

    const chat_gaming::room &r = self_model()->hosting_room();
    // Duplicate the message for sending back a reply (with different id)
    message_room_join *smsg =
      dynamic_ptr_cast<message_room_join>(m->duplicate());

    // First see if self is even hosting the room in question!
    if ((self_model()->user().room_id() != r.id() ||
        (self_model()->user().room_id() != smsg->room_id())))
    {
        ACE_DEBUG((LM_DEBUG, "house_app::_handle_room_join: "
                  "received a request to join room self is not hosting. "
                  "Self hosting room id '%s', self in room '%s', request for "
                  "room %s received\n",
                  r.id().c_str(),
                  self_model()->user().room_id().c_str(),
                  smsg->room_id().c_str()));
        smsg->id(message::room_join_nack);
        net_messenger()->send_msg(smsg);
        return;
    }
    // If password set for the room we're hosting, check that the user
    // gave the right one.
    if (!r.password().empty() && r.password() != m->password()) {
        ACE_DEBUG((LM_DEBUG, "house_app::_handle_room_join: invalid password, "
                  "the hosted room requires password %s\n",
                  r.password().c_str()));
        smsg->id(message::room_join_pass);
        net_messenger()->send_msg(smsg);
        return;
    }

    size_t room_size = house_model()->room_size(smsg->room_id());
    ACE_DEBUG((LM_DEBUG, "house_app::_handle_room_join: "
              "room has %d players of maximum of %d\n",
              room_size, r.max_players()));
    if (room_size >= r.max_players()) {
        ACE_DEBUG((LM_DEBUG, "house_app::_handle_room_join: full room, "
                  "room has %d players of maximum of %d\n",
                  room_size, r.max_players()));
        smsg->id(message::room_join_full);
        net_messenger()->send_msg(smsg);
        return;
    }

    // Temporarily change the joining user to be in this room, to reserve
    // the place for the player until his client receives our answer and
    // can change the status himself.
    model::house::house_type::user_iterator ui =
      house_model()->user_find(smsg->sender_id(), smsg->group_base());
    if (ui != house_model()->user_end()) {
        ui->room_id(smsg->room_id());
    } else {
        ACE_ERROR((LM_ERROR, "house_app::_handle_room_join: could not find "
                  "matching user for id '%s' from group %d\n",
                  smsg->sender_id().c_str(), smsg->group_base()));
    }

    ACE_DEBUG((LM_DEBUG, "house_app::_handle_room_join: sending join ack\n"));

    smsg->id(message::room_join_ack);
    net_messenger()->send_msg(smsg);
}

void
house_app::_handle_room_join_ack(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "house_app::_handle_room_join_ack\n"));

    if (self_model()->joining_room().empty()) {
        ACE_DEBUG((LM_DEBUG, "house_app::_handle_room_join_ack not joining "
                  "currently any room, message ignored\n"));
        return;
    }

    const char *topic   = langstr("app/join_err_topic");
    const char *content = NULL;

    switch(msg->id()) {
    case ::message::room_join_full:
        content = langstr("app/join_room_full");
        break;
    case ::message::room_join_pass:
        content = langstr("app/join_invalid_pass");
        break;
    case ::message::room_join_nack:
        content = langstr("app/join_refused");
        break;
    case ::message::room_join_ack:
        // Change self state to be in the room we tried to join
        // TODO check that the join ack was to the room we were trying to
        // join!
        if (!self_model()->joining_room().empty()) {
            self_model()->user().room_id(self_model()->joining_room());
            message *msg = new message_user(
                ::message::user,
                self_model()->user(),
                self_model()->sequence(),
                0
              );
            net_messenger()->send_msg(msg);
            self_model()->joining_room(chat_gaming::room::id_type());
        }
        break;
    }

    if (content) {
        ACE_DEBUG((LM_DEBUG, "house_app::_handle_room_join_ack: %s\n", content));
        // Set not joining anymore and display the error
        self_model()->joining_room(chat_gaming::room::id_type());

        QMessageBox msg_box(_house_win);
        msg_box.setWindowTitle(topic);
        msg_box.setText(content);
        msg_box.setIcon(QMessageBox::Information);
        msg_box.exec();
    }

    interruptable_action_update();
}

void
house_app::_handle_global_ignore(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "house_app::_handle_global_ignore\n"));
    message_global_ignore *m = dynamic_ptr_cast<message_global_ignore>(msg);
    message_global_ignore::ip_list_type::const_iterator i = m->ip_begin();

    message_block_users *mb = new message_block_users(
        ::message::block_users,
        self_model()->user(),
        self_model()->sequence(),
        0
    );

    mb->global_ignore(true);

    auto_ptr<message_block_users> mb_guard(mb);

    for (; i != m->ip_end(); i++) {
        const std::string &ipstr   = i->ip;
        const std::string &maskstr = i->mask;
        struct in_addr a;
        struct in_addr m;
        if (!ACE_OS::inet_aton(ipstr.c_str(),   &a)   ||
            !ACE_OS::inet_aton(maskstr.c_str(), &m))
        {
            ACE_DEBUG((LM_ERROR, "house_app::global_ignore invalid IP "
                      " or mask address: %s/%s\n",
                      ipstr.c_str(), maskstr.c_str()));
            continue;
        }

        a.s_addr = ntohl(a.s_addr);
        m.s_addr = ntohl(m.s_addr);

        if (a.s_addr == INADDR_ANY      ||
            a.s_addr == INADDR_LOOPBACK ||
            a.s_addr == INADDR_NONE     ||
            m.s_addr == INADDR_ANY)
        {
            ACE_DEBUG((LM_ERROR, "house_app::global_ignore IP "
                      " or mask invalid as IP: %s/%s\n",
                      ipstr.c_str(), maskstr.c_str()));
            continue;
        }

        ACE_DEBUG((LM_DEBUG, "house_app::global_ignore recognized IP\n"));
        mb->ip_push_back(a.s_addr, m.s_addr);
    }

    if (mb->ip_size() > 0) {
        // House window is also interested in this message, so pass it
        // there too.
        _house_win->handle_message(mb);
        net_messenger()->send_msg(mb_guard.release());
    }
}

// Returns true if the message should be disregarded
bool
house_app::_filter_message(::message *msg) {
    // Can only filter messages that are group based.
    message_grouped *m = dynamic_cast<message_grouped *>(msg);
    // 0 is special sequence that is used only on messages that have no
    // clear sender but originate from the system - thus they can't be
    // effectively filtered for duplicates
    if (!m || m->sequence() == 0) return false;

    // Then check that the message is one of the messages that are to
    // be filtered. Messages that affect the model are not subjected to
    // filtering since the model has to be kept up to date regarding all
    // group feeds. Other messages should be filtered.

    switch (m->id()) {
    case message::user:
    case message::user_left:
    case message::room:
    case message::room_remove:
        return false;
    default:
        return _filter_dup(m->sender_id().id_str(),
                           m->sequence(),
                           &_filter_dup_map,
                           m->group_base());
    }

    return false;
}

// Filter's away channel messages that have already been received
bool
house_app::_filter_dup(
    const std::string &sndr_id, unsigned seq,
    _filter_dup_map_type *map,
    int group_base)
{
    if (sndr_id.empty()) {
        ACE_DEBUG((LM_DEBUG, "house_app::_filter_dup sndr_id empty???"
                   "From group %d\n", group_base));
        return false;
    }

    _filter_dup_map_type::iterator i = map->find(sndr_id);

    if (i != map->end()) {
        _filter_dup_struct &uss = i->second;
        // Filter away this message if a matching sequence already been
        // seen:
        if (std::find(uss.seqs.begin(), uss.seqs.end(), seq) !=
            uss.seqs.end())
        {
            ACE_DEBUG((LM_DEBUG, "house_app::_filter_dup seq %u from user %s "
                      "already seen, skipping\n",
                      seq, sndr_id.c_str()));
            return true;
        }

        // Insert the sequence to the list of sequences seen.
        uss.seqs.push_front(seq);
        ACE_DEBUG((LM_DEBUG, "house_app::_filter_dup new seq %d from user %s, "
                  "size of seq list now %d\n", seq, sndr_id.c_str(),
                  uss.seqs.size()));
        if (uss.seqs.size() > _filter_dup_struct::max_seqs) {
            unsigned lseq = uss.seqs.back();
            uss.seqs.pop_back();
            ACE_DEBUG((LM_DEBUG, "house_app::_filter_channel removed last seq %d, "
                      "size now %d\n", lseq, uss.seqs.size()));
        }
    } else {
        ACE_DEBUG((LM_DEBUG, "house_app::_filter_channel inserting new user "
                  "%s, new sequence %d\n", sndr_id.c_str(),
                  seq));
        _filter_dup_struct &uss = (*map)[sndr_id];
        uss.seqs.push_front(seq);
    }

    return false;
}

// IMRPOVE Qt test
void
house_app::_handle_worker_exception(message *om) {
    message_string *m = dynamic_ptr_cast<message_string>(om);

    ACE_DEBUG((LM_DEBUG, "house_app::worker_exception: %s\n", m->str().c_str()));

    QMessageBox::critical(_house_win, "Fatal error", m->str().c_str());

    delete _net_worker;
    _net_worker = NULL;
    // net_messenger.instance(NULL);
    this->quit();
}

void
house_app::_version_update_window(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "house_app::version_update_window\n"));
    message_version *m = dynamic_ptr_cast<message_version>(msg);
    window::version_update *win = new window::version_update(m, _house_win);
    win->show_when_possible();
    ACE_DEBUG((LM_DEBUG, "house_app::version_update_window done\n"));
}

void
house_app::_handle_model_update(model::house *hm, ::message *msg) {
    _handle_user_model_update(hm, msg);
    _handle_room_model_update(hm, msg);
}

void
house_app::_handle_user_model_update(model::house *hm, ::message *msg) {
    // not perhaps the best or most efficient way to do this, but nevermind
    message_user *m = dynamic_cast<message_user *>(msg);
    if (!m) return;

    switch (m->id()) {
    case message::user:
        hm->user_update(m->user(), m->group_base());
        break;
    case message::user_left:
        hm->user_erase(m->user().id(), m->group_base());
        break;
    }
}

void
house_app::_handle_room_model_update(model::house *hm, ::message *msg) {
    // not perhaps the best or most efficient way to do this, but nevermind
    message_room *m = dynamic_cast<message_room *>(msg);
    if (!m) return;

    switch (m->id()) {
    case message::room:
    {
        const chat_gaming::room &r = m->room();
        hm->room_update(r, m->group_base());
    }
        break;
    case message::room_remove:
        hm->room_erase(m->room().id(), m->group_base());
        break;
    }
}

// TODO logic of not opening a window if self is not joining_room
void
house_app::_cond_open_room_window(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "house_app::_cond_open_room_window\n"));
    if (_room_win) return;

    message_grouped *mg = dynamic_cast<message_grouped *>(msg);
    if (!mg) return;

    ACE_DEBUG((LM_DEBUG, "house_app: find self\n"));

    model::house::house_type::user_iterator self;
    model::house::house_type::room_iterator room;

    self = house_model()->user_find(self_model()->user().id(), mg->group_base());

    ACE_DEBUG((LM_DEBUG, "house_app: self found\n"));

    if (self == house_model()->user_end()) {
        ACE_DEBUG((LM_DEBUG, "house_app::room_update self not found for "
                  "user id %s, group_base %d\n",
                  self_model()->user().id().c_str(), mg->group_base()));
        return;
    }

    if (self->room_id().empty()) {
        ACE_DEBUG((LM_DEBUG, "house_app::room_update self not in any rooms, "
                   "cond room open not continued\n"));
        return;
    }

    room = house_model()->room_find(self->room_id(),  mg->group_base());

    if (room == house_model()->room_end()) {
        ACE_DEBUG((LM_DEBUG, "house_app::room_update self not found for "
                  "user id %s, group_base %d\n",
                  self_model()->user().id().c_str(), mg->group_base()));
        return;
    }

    ACE_DEBUG((LM_DEBUG, "house_app: contemplate message\n"));

    switch(msg->id()) {
    case ::message::room:
    case ::message::user:
        break;
    default:
        ACE_DEBUG((LM_DEBUG, "house_app: not room or user message, return\n"));
        return;
    }

    ACE_DEBUG((LM_DEBUG, "house_app: wonder about stuff\n"));

    // Check if there is need to create a room window, ie. if
    // the self user's room id equals the updated ones and
    // the topic is set
    ACE_DEBUG((LM_DEBUG, "room id: %s self in room id: %s, room topic: %s\n",
              room->id().c_str(), self->room_id().c_str(), room->topic().c_str()));
    if (!_room_win && !room->topic().empty()) {
        ACE_DEBUG((LM_DEBUG, "Creating room window\n"));
        _room_win = new window::room(room->id(), _house_win);
        _room_win->setAttribute(Qt::WA_DeleteOnClose);
        _room_win->show();
        _room_win->update(mg->group_base());
    }
}

void
house_app::_cond_open_private_message_window(::message *msg) {
    message_channel *m = dynamic_cast<message_channel *>(msg);
    if (!m) return;

    bool allow_new_window  = true;
    bool new_window_needed = false;

    ACE_DEBUG((LM_DEBUG, "house_app::_cond_open_private_message_window, self user_id %s, message sender id %s\n",
            self_model()->user().id().id_str().c_str(), m->sender_id().id_str().c_str()));

    if (self_model()->user().status() == chat_gaming::user::status_playing     ||
        self_model()->user().status() == chat_gaming::user::status_dont_disturb) {
        allow_new_window = false;
    }

    window::private_message *win = private_message_window_to(
        m->channel(),
        m->sender_id().id_str(),
        false,
        allow_new_window,
        &new_window_needed
    );

    if (!allow_new_window && new_window_needed) {
        message_channel *smsg =
          dynamic_ptr_cast<message_channel>(m->duplicate());

        smsg->id(::message::private_refused);
        smsg->sequence(self_model()->sequence());
        net_messenger()->send_msg(smsg);
    }
    else if (win && new_window_needed) {
        win->handle_message(msg);
    }
}

window::private_message *
house_app::private_message_window_to(
    const std::string &channel,
    const std::string &user_id_str,
    bool raise,
    bool allow_new_window,
    bool *new_window_needed
) {
    ACE_DEBUG((LM_DEBUG, "house_app::private_message_window_for(%s, %s, %d)\n",
              channel.c_str(), user_id_str.c_str(), raise));

    if (self_model()->user().id().id_str() == user_id_str) {
        ACE_DEBUG((LM_DEBUG, "house_app::private_message_window_to tried opening private message to self not allowed\n"));
        return NULL;
    }

    // Try finding a matching window first
    window::private_message *win = NULL;
    for (_watched_list_type::iterator wi = _watched_list.begin();
         wi != _watched_list.end(); wi++)
    {
        win = dynamic_cast<window::private_message *>(*wi);
        ACE_DEBUG((LM_DEBUG, "house_app::private_message win search, win %d, win_channel %s, channel %s\n",
                  win, (win == NULL ? NULL : win->channel().c_str()), channel.c_str()));

        if (win && win->channel() == channel) break;
        win = NULL;
    }

    if (win) {
        // A private message window already exists, bring to top
        if (raise)
            win->raise();
        else {
            os::flash_window(win); // win->setFocus();
        }
        if (new_window_needed) *new_window_needed = false;

        return win;
    }

    if (new_window_needed) *new_window_needed = true;

    if (!allow_new_window) {
        return NULL;
    }

    win = new window::private_message(user_id_str);
    win->setAttribute(Qt::WA_DeleteOnClose);
    win->show();

    // Must emulate the addition of participants of private chat
    std::vector<std::string> users(2);
    std::vector<std::string>::const_iterator i = users.begin();

    users[0] = user_id_str;
    users[1] = self_model()->user().id().id_str();

    for (; i != users.end(); i++) {
        ACE_DEBUG((LM_DEBUG, "private_message_window_for: iterating user %s\n",
                  i->c_str()));
        const model::house::group_desc_type &g = house_model()->group_desc();
        model::house::group_desc_type::const_iterator gi = g.begin();
        for (; gi != g.end(); gi++) {
            model::house::user_iterator user_i
              = house_model()->user_find(*i, gi->first);
            if (user_i == house_model()->user_end()) continue;

            std::auto_ptr<message_user> m (
              new message_user(::message::user, *user_i,
                               0, gi->first)
            );
            ACE_DEBUG((LM_DEBUG, "private_message_window_for: handling message\n"));
            win->handle_message(m.get());
        }
    }

    return win;
}

} // ns gui
