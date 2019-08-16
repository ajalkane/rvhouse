#include <utility>
#include <iterator>

#include <QtGui>

#include "house.h"

#include "room_settings.h"
#include "router_fw_help.h"
#include "settings.h"
#include "about.h"

#include "../house_app.h"
#include "../util/flood_control.h"
#include "../util/util.h"
#include "../../app_options.h"
#include "../../app_version.h"
#include "../../common.h"
#include "../../icon_store.h"
#include "../../main.h"
#include "../../os_util.h"
#include "../../executable/launcher.h"
#include "../../messaging/message_block_users.h"
#include "../../messaging/message_user.h"

#include "../../messaging/message_room.h"
#include "../../messaging/message_send.h"
#include "../../messaging/message_channel.h"
#include "../../messaging/messenger.h"
#include "../../model/house.h"
#include "../../model/self.h"

// IMPROVE Qt these do not belong here, should be contained inside users
#include "../../multi_feed/util.h"
#include "../../multi_feed/algorithm.h"
#include "../../multi_feed/user_accessors.h"

#define STRING_JOIN(target, sep, src) \
target = (target.empty() ? src : (src.empty() ? target : target + sep + src))

namespace gui {
namespace window {

house::house()
    : size_restoring_window<QMainWindow>("house_win"),
      _status_dht_extra(2),
      _last_connect(0),
      _conn_tries(0),
      _ctz_disconnected(true),
      _last_dht_status_message_id(0),
      _router_fw_help_showed(false)
{
    ACE_DEBUG((LM_DEBUG, "house::house\n"));
    this->setWindowTitle(APP_NAME);
    this->setWindowIcon(app_icons()->get("rv_house"));
    this->setContextMenuPolicy(Qt::NoContextMenu);

    // Instance must be set here in constructor because users constructor needs it
    house_win.instance(this);
    _create_actions();
    _create_menus();
    _create_toolbars();
    _create_widgets();
    _create_layout();

    _connect_signals();

    _update_status();
    _room_buttons_status();

    interruptable_action_update();
}

void
house::_create_actions() {
    _action_quit = new QAction(app_icons()->get("exit"), langstr("menu_file/quit"), this);
    _action_quit->setShortcut(QKeySequence::Quit);

    _action_settings = new QAction(langstr("menu_edit/settings"), this);
    _action_settings->setShortcut(Qt::ALT | Qt::Key_S);

    _action_status = new QAction(langstr("menu_player/status"), this);
    _action_status->setDisabled(true);

    _actiongroup_status = new QActionGroup(this);
    _action_chatting = new QAction(langstr("menu_player/chatting"), this);
    _action_chatting->setCheckable(true);
    _action_chatting->setShortcut(Qt::ALT | Qt::SHIFT | Qt::Key_C);
    _action_playing = new QAction(langstr("menu_player/playing"), this);
    _action_playing->setCheckable(true);
    _action_playing->setShortcut(Qt::ALT | Qt::SHIFT | Qt::Key_P);
    _action_away = new QAction(langstr("menu_player/away"), this);
    _action_away->setShortcut(Qt::ALT | Qt::SHIFT | Qt::Key_A);
    _action_away->setCheckable(true);
    _action_donotdisturb = new QAction(langstr("menu_player/dont_disturb"), this);
    _action_donotdisturb->setCheckable(true);
    _action_donotdisturb->setShortcut(Qt::ALT | Qt::SHIFT | Qt::Key_D);
    _actiongroup_status->addAction(_action_chatting);
    _actiongroup_status->addAction(_action_playing);
    _actiongroup_status->addAction(_action_away);
    _actiongroup_status->addAction(_action_donotdisturb);
    _action_chatting->setChecked(true);

    _action_rvio_online = new QAction(langstr("menu_comp/rvio"), this);
    _action_rvr_home = new QAction(langstr("menu_comp/rvr"), this);
    _action_rvr_best3_laps = new QAction(langstr("menu_comp/rvr_best3"), this);
    _action_rvr_best4_laps = new QAction(langstr("menu_comp/rvr_best4"), this);
    _action_rvr_month_tracks = new QAction(langstr("menu_comp/rvr_month"), this);

    _action_dload_rvgl = new QAction(langstr("menu_dloads/rvgl"), this);
    //_action_dload_rv12 = new QAction(langstr("menu_dloads/rv12"), this);
    _action_dload_rvzone = new QAction(langstr("menu_dloads/rvzone"), this);
    _action_dload_rvio = new QAction(langstr("menu_dloads/rvio"), this);
    _action_dload_xtg = new QAction(langstr("menu_dloads/xtg"), this);
    //_action_dload_jigebren = new QAction(langstr("menu_dloads/jigebren"), this);

    _action_routerfw_help = new QAction(langstr("menu_help/router_help"), this);
    _action_rvh_faq = new QAction(langstr("menu_help/faq"), this);
    _action_rv_wiki = new QAction(langstr("menu_help/wiki"), this);
    _action_trh_forum = new QAction(langstr("menu_help/trh_forum"), this);
    //_action_pub_forum = new QAction(langstr("menu_help/pub_forum"), this);
    //_action_rvl_forum = new QAction(langstr("menu_help/rvl_forum"), this);
    _action_about_rvh = new QAction(app_icons()->get("about"), langstr("menu_help/about_house"), this);

    _action_create_room = new QAction(app_icons()->get("room_create"), langstr("menu_player/create_room"), this);
    _action_create_room->setShortcut(Qt::CTRL | Qt::Key_R);
    _action_join_room   = new QAction(app_icons()->get("room_join"), langstr("main_win/join_room"), this);
    _action_refresh     = new QAction(app_icons()->get("refresh"), langstr("main_win/refresh"), this);
    _action_cancel      = new QAction(app_icons()->get("cancel"), langstr("main_win/cancel"), this);

}

void
house::_create_menus() {
    // File menu
    _menu_file = new QMenu(langstr("menu_file/title"), this);
    _menu_file->addAction(_action_quit);

    // Edit menu
    _menu_edit = new QMenu(langstr("menu_edit/title"), this);
    _menu_edit->addAction(_action_settings);
    // Player menu
    _menu_player = new QMenu(langstr("menu_player/title"), this);
    _menu_player->addAction(_action_status);
    _menu_player->addSeparator();
    _menu_player->addActions(_actiongroup_status->actions());
    _menu_player->addSeparator();
    _menu_player->addAction(_action_create_room);

    // Competitions menu
    _menu_comp = new QMenu(langstr("menu_comp/title"), this);
    _menu_comp->addAction(_action_rvio_online);
    _menu_comp->addAction(_action_rvr_home);
    _menu_comp->addSeparator();
    _menu_comp->addAction(_action_rvr_best3_laps);
    _menu_comp->addAction(_action_rvr_best4_laps);
    _menu_comp->addAction(_action_rvr_month_tracks);

    // Downloads menu
    _menu_dloads = new QMenu(langstr("menu_dloads/title"), this);
    _menu_dloads->addAction(_action_dload_rvgl);
    //_menu_dloads->addAction(_action_dload_rv12);
    _menu_dloads->addAction(_action_dload_rvzone);
    _menu_dloads->addAction(_action_dload_rvio);
    _menu_dloads->addAction(_action_dload_xtg);
    //_menu_dloads->addAction(_action_dload_jigebren);

    // Help menu
    _menu_help = new QMenu(langstr("menu_help/title"), this);
    _menu_help->addAction(_action_routerfw_help);
    _menu_help->addAction(_action_rvh_faq);
    _menu_help->addAction(_action_rv_wiki);
    _menu_help->addAction(_action_trh_forum);
    //_menu_help->addAction(_action_pub_forum);
    //_menu_help->addAction(_action_rvl_forum);
    _menu_help->addSeparator();
    _menu_help->addAction(_action_about_rvh);

    menuBar()->addMenu(_menu_file);
    menuBar()->addMenu(_menu_edit);
    menuBar()->addMenu(_menu_player);
    menuBar()->addMenu(_menu_comp);
    menuBar()->addMenu(_menu_dloads);
    menuBar()->addMenu(_menu_help);
}

void
house::_create_toolbars() {
    QToolBar *tool_bar = addToolBar("Quit");
    tool_bar->setObjectName("toolbar_quit");
    tool_bar->addAction(_action_quit);
    tool_bar = addToolBar("Room controls");
    tool_bar->setObjectName("toolbar_roomcontrols");
    tool_bar->addAction(_action_create_room);
    tool_bar->addAction(_action_join_room);
    tool_bar->addAction(_action_refresh);
    tool_bar->addAction(_action_cancel);
    tool_bar = addToolBar("Miscellaneous");
    tool_bar->setObjectName("toolbar_misc");
    tool_bar->addAction(_action_about_rvh);
}

void
house::_create_widgets() {
    _chat_view  = new view::chat(this);
    _users_view = new view::users(this);
    _rooms_view = new view::rooms(this);
    _msg_field  = new QLineEdit(this);
    _msg_field->setMaxLength((int)app_opts.limit_chat_msg());

    _rooms_view->observer_set(this);
    _users_view->observer_set(this);
    _chat_view->flash_window(this);

    // This ensures message field keeps the focus even if
    // user scrolls or otherwise operates the other widgets.
    // Is there a better way to do this?
    _chat_view->setFocusProxy(_msg_field);
    _users_view->setFocusProxy(_msg_field);
    _rooms_view->setFocusProxy(_msg_field);
}

void
house::_connect_signals() {
    connect(_action_quit,   SIGNAL(triggered()), app(), SLOT(quit()));
    connect(_msg_field,     SIGNAL(returnPressed()), this, SLOT(send_message()));
    connect(_action_cancel, SIGNAL(triggered()), this, SLOT(interrupt()));

    connect(_action_chatting,     SIGNAL(triggered()), this, SLOT(change_user_status_to_chatting()));
    connect(_action_away,         SIGNAL(triggered()), this, SLOT(change_user_status_to_away()));
    connect(_action_playing,      SIGNAL(triggered()), this, SLOT(change_user_status_to_playing()));
    connect(_action_donotdisturb, SIGNAL(triggered()), this, SLOT(change_user_status_to_dont_disturb()));

    connect(_action_create_room, SIGNAL(triggered()), this, SLOT(room_create()));
    connect(_action_join_room,   SIGNAL(triggered()), this, SLOT(room_join_selected()));
    connect(_action_refresh,     SIGNAL(triggered()), this, SLOT(refresh()));

    connect(_action_settings,  SIGNAL(triggered()), this, SLOT(open_settings()));

    connect(_action_about_rvh, SIGNAL(triggered()), this, SLOT(open_about()));
    connect(_rooms_view,       SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(room_join_selected(QTreeWidgetItem *, int)));
    connect(_users_view,       SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(open_private_room(QTreeWidgetItem *, int)));

    QSignalMapper *openUrlSignalMapper = new QSignalMapper(this);
    openUrlSignalMapper->setMapping(_action_rvio_online,      conf()->get_value("www/rvio_online").c_str());
    openUrlSignalMapper->setMapping(_action_rvr_home,         conf()->get_value("www/rvr_home").c_str());
    openUrlSignalMapper->setMapping(_action_rvr_best3_laps,   conf()->get_value("www/rvr_best3").c_str());
    openUrlSignalMapper->setMapping(_action_rvr_best4_laps,   conf()->get_value("www/rvr_best4").c_str());
    openUrlSignalMapper->setMapping(_action_rvr_month_tracks, conf()->get_value("www/rvr_month").c_str());

    openUrlSignalMapper->setMapping(_action_dload_rvgl,     conf()->get_value("www/dloads_rvgl").c_str());
    //openUrlSignalMapper->setMapping(_action_dload_rv12,     conf()->get_value("www/dloads_rv12").c_str());
    openUrlSignalMapper->setMapping(_action_dload_rvzone,   conf()->get_value("www/dloads_rvz").c_str());
    openUrlSignalMapper->setMapping(_action_dload_rvio,     conf()->get_value("www/dloads_rvio").c_str());
    openUrlSignalMapper->setMapping(_action_dload_xtg,      conf()->get_value("www/dloads_xtg").c_str());
    //openUrlSignalMapper->setMapping(_action_dload_jigebren, conf()->get_value("www/dloads_jig").c_str());

    openUrlSignalMapper->setMapping(_action_routerfw_help,  conf()->get_value("www/help_router").c_str());
    openUrlSignalMapper->setMapping(_action_rvh_faq,        conf()->get_value("www/help_faq").c_str());
    openUrlSignalMapper->setMapping(_action_rv_wiki,        conf()->get_value("www/help_wiki").c_str());
    openUrlSignalMapper->setMapping(_action_trh_forum,      conf()->get_value("www/forum_trh").c_str());
    //openUrlSignalMapper->setMapping(_action_pub_forum,      conf()->get_value("www/forum_pub").c_str());
    //openUrlSignalMapper->setMapping(_action_rvl_forum,      conf()->get_value("www/forum_rvl").c_str());

    connect(_action_rvio_online,      SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));
    connect(_action_rvr_home,         SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));
    connect(_action_rvr_best3_laps,   SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));
    connect(_action_rvr_best4_laps,   SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));
    connect(_action_rvr_month_tracks, SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));

    connect(_action_dload_rvgl,      SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));
    //connect(_action_dload_rv12,      SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));
    connect(_action_dload_rvzone,    SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));
    connect(_action_dload_rvio,      SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));
    connect(_action_dload_xtg,       SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));
    //connect(_action_dload_jigebren,  SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));

    connect(_action_routerfw_help,   SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));
    connect(_action_rvh_faq,         SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));
    connect(_action_rv_wiki,         SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));
    connect(_action_trh_forum,       SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));
    //connect(_action_pub_forum,       SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));
    //connect(_action_rvl_forum,       SIGNAL(triggered()), openUrlSignalMapper, SLOT(map()));

    connect(openUrlSignalMapper, SIGNAL(mapped(QString)), this, SLOT(open_url(QString)));

}

void
house::_create_layout() {
    QVBoxLayout *l = new QVBoxLayout;
    _chat_users_splitter = new QSplitter(Qt::Horizontal);
    QSplitter *vsplitter = new QSplitter(Qt::Vertical);
    vsplitter->addWidget(_rooms_view);
    vsplitter->addWidget(_chat_view);
    _chat_users_splitter->addWidget(vsplitter);
    _chat_users_splitter->addWidget(_users_view);

    l->addWidget(_chat_users_splitter);
    l->addWidget(_msg_field);

    vsplitter->setStretchFactor(1, -1);
    _chat_users_splitter->setStretchFactor(0, -1);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(l);
    this->setCentralWidget(centralWidget);

    _msg_field->setFocus();
}

void
house::show() {
    super::show();

    QList<int> _chat_users_splitter_sizes = _chat_users_splitter->sizes();
    _chat_users_splitter_sizes.back() = 150;
    _chat_users_splitter->setSizes(_chat_users_splitter_sizes);
}

house::~house() {
    house_win.instance(NULL);
}

void
house::change_user_status_to_chatting() {
    _change_user_status(chat_gaming::user::status_chatting);
}

void
house::change_user_status_to_playing() {
    _change_user_status(chat_gaming::user::status_playing);
}

void
house::change_user_status_to_away() {
    _change_user_status(chat_gaming::user::status_away);
}

void
house::change_user_status_to_dont_disturb() {
    _change_user_status(chat_gaming::user::status_dont_disturb);
}

void
house::_change_user_status(chat_gaming::user::status_enum user_status) {
    ACE_DEBUG((LM_DEBUG, "house::_change_user_status: %d\n\n", user_status));

    // Restrict status changing so that it can be done only once in a
    // second
    static time_t last_change = 0;
    time_t now = time(NULL);
    if (now - last_change < 1) {
        ACE_DEBUG((LM_DEBUG, "house::_change_user_status: not done, now - last_change = %d\n\n", now - last_change));
        return;
    }

    if (user_status != self_model()->user().status()) {
        ACE_DEBUG((LM_DEBUG, "house::_change_user_status: done, user_send\n"));
        last_change = now;
        self_model()->user().status(user_status);
        self_model()->user_send();
    }
}

void
house::send_message() {
    QString t = _msg_field->text();

    if (t.isEmpty()) return;

    if (!_flood_control.allow_send()) {
        _chat_view->status_message(langstr("chat/flood_control"));
        return;
    }
    if (t.length() > (int)app_opts.limit_chat_msg())
        t.truncate(app_opts.limit_chat_msg());

    // IMPROVE Qt check if this needed
    // t.substitute("\r", "");


    // Check for a command
    if (!_chat_command(t)) {
        // Sending a public message
        message_send *msg =
          new message_send(::message::send,
                           t.toLatin1().constData(),
                           self_model()->user().id(),
                           self_model()->sequence(),
                           0);
        net_messenger()->send_msg(msg);
    }

    _msg_field->setText("");

    emit send_message();
}

bool
house::_chat_command(const QString &t) {
    if (t[0] == '/') {
        std::string cmd = t.toLatin1().constData() + 1;
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

void
house::net_connect() {
    chat_gaming::user u(self_model()->user());

    ACE_DEBUG((LM_DEBUG, "TODO debug user id str: %s\n",
              self_model()->user().id().id_str().c_str()));
    ::message *msg = new message_user(::message::connect,
        self_model()->user(),
        self_model()->sequence(), 0);
    _conn_tries++;
    _last_connect = time(NULL);

    net_messenger()->send_msg(msg);
}

void
house::net_disconnect() {

    ::message *msg = new ::message(::message::disconnect);
    net_messenger()->send_msg(msg);
}

void
house::room_create() {
    ACE_DEBUG((LM_DEBUG, "house: creating room\n"));

    // TODO Don't allow creating a new room if the user is already in another
    // room.
    window::room_settings *win = new window::room_settings(this);

    int ret_value = win->exec();
    if (ret_value == QDialog::Rejected) {
        ACE_DEBUG((LM_DEBUG, "house::Room create cancelled\n"));
        return;
    }

    ACE_DEBUG((LM_DEBUG, "house::Room create successfull\n"));
}

void
house::room_join_selected() {
    ACE_DEBUG((LM_DEBUG, "house::room_join_selected\n"));
    QTreeWidgetItem *item = _rooms_view->currentItem();
    ACE_DEBUG((LM_DEBUG, "house::room_join_selected currentItem %d\n", item));
    room_join_selected(item, 0);
}

void
house::room_join_selected(QTreeWidgetItem *item, int column) {
    ACE_DEBUG((LM_DEBUG, "house: joining room\n"));

    if (item == NULL) {
        ACE_DEBUG((LM_DEBUG, "house: no room selected\n"));
        return;
    }

    if (item->isDisabled()) {
        ACE_DEBUG((LM_DEBUG, "house: room is disabled\n"));
        return;
    }

    if (!self_model()->user().room_id().empty()) {
        ACE_DEBUG((LM_DEBUG, "house: already in room id %s, ignored\n",
                   self_model()->user().id().c_str()));
        return;
    }
    if (!self_model()->joining_room().empty()) {
        ACE_DEBUG((LM_DEBUG, "house: already joining room id %s, ignored\n",
                   self_model()->joining_room().c_str()));
        return;
    }

    view::rooms::item_type *selected_item = static_cast<view::rooms::item_type *>(item);
    ACE_DEBUG((LM_DEBUG, "house::selected_item is %d\n", selected_item));

    if (selected_item == NULL) {
        return;
    }

    // room_item *item = _rooms_view->item_at(i);
    chat_gaming::room::id_type rid = selected_item->room_id();

    if (rid.empty()) {
        ACE_DEBUG((LM_ERROR, "house::on_room_join no matching room "
                  "found for room pointer %d\n", selected_item));
        return;
    }

    model::house::house_type::room_iterator ri = house_model()->room_find(rid);
    if (ri == house_model()->room_end()) {
        ACE_DEBUG((LM_ERROR, "house::on_room_join no matching room "
                  "found from model for room id %s\n", rid.c_str()));
        return;
    }

    chat_gaming::room r(*ri);
    // Check if the room requires a password and ask for it if so
    if (r.has_password()) {
        bool ok = false;
        QString pass = QInputDialog::getText(this,
                                             langstr("main_win/ask_room_pass_title"),
                                             langstr("main_win/ask_room_pass"),
                                             QLineEdit::Normal, QString(),
                                             &ok);
        if (!ok) {
            return;
        }
        r.password(pass.toLatin1().constData());
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
}

// IMPROVE Qt: I think this would fit better into house_app().cpp
void
house::open_private_room(QTreeWidgetItem *widget_item, int column) {
    ACE_DEBUG((LM_DEBUG, "house::open_private_room\n"));

    view::users::item_type *item = dynamic_cast<view::users::item_type *>(widget_item);

    if (!item) {
        ACE_DEBUG((LM_DEBUG, "house::open_private_room not a room item\n"));
        return;
    }

    // IMPROVE using multi_feed does not belong here, it should be contained within users.
    std::string user_id = multi_feed::value(
        item->feed_item(),
        multi_feed::user_id_accessor(multi_feed::user_id_to_id_string())
    );

    ACE_DEBUG((LM_DEBUG, "users::on_priv_msg: got '%s'\n",
              user_id.c_str()));

    if (!user_id.empty()) {
        ::app()->private_message_window_to(
            util::private_message_channel_with(user_id),
            user_id,
            true
        );
    }
}

void
house::open_settings() {
    ACE_DEBUG((LM_DEBUG, "house::open_settings\n"));

    settings *settings_win = new settings;
    settings_win->setAttribute(Qt::WA_DeleteOnClose);
    settings_win->show();
}

void
house::open_url(const QString &url) {
    const char *urlstr = url.toLatin1().constData();
    ACE_DEBUG((LM_DEBUG, "house::open_url starting %s\n", urlstr));
   launcher_file()->start(urlstr);

}

void
house::open_about() {
    ACE_DEBUG((LM_DEBUG, "house::open_about\n"));

    gui::window::about about(this);

    about.exec();
}

void
house::handle_message(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "house: handle_message\n"));

    if (handle_dht_message(msg)         ||
        handle_ctz_message(msg)         ||
        handle_external_ip_message(msg))
    {
        _update_status();
        return;
    }

    switch(msg->id()) {
    case ::message::room:
        _room_buttons_status();
        break;
    case ::message::user:
    {
        _room_buttons_status();
    }
        break;
    case ::message::send:
        // Don't flash the window, however, if playing
        // already
        ACE_DEBUG((LM_DEBUG, "house::handle_message: flash_main_chat %d\n",
                  app_opts.flash_main_chat()));
        if (app_opts.flash_main_chat() &&
            self_model()->user().status() != chat_gaming::user::status_playing)
        {
            message_channel *m = dynamic_ptr_cast<message_channel>(msg);
            ACE_DEBUG((LM_DEBUG, "house::handle_message send: m->channel() %s, _chat_view->channel() %s\n",
                    m->channel().c_str(), _chat_view->channel().c_str()));
            if (m->channel() == _chat_view->channel()) {
                ACE_DEBUG((LM_DEBUG, "house::handle_message send flashing main window\n"));
                os::flash_window(this);
            }
        }
        break;
    case ::message::block_users:
    {
        message_block_users *mb = dynamic_ptr_cast<message_block_users>(msg);
        if (mb->global_ignore()) {
            handle_global_block_users(mb);
        }
    }
        break;

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
        QTimer::singleShot(5000, this, SLOT(dht_disconnected()));
    }

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

        // NOTE(huki): I've disabled limit on retry attempts as we would
        // need to attempt a reconnection in case the server disconnects
        // the user because of inactivity.
        //if (_conn_tries < 5) {
        int conn_in_secs = 30; // * _conn_tries;
        ACE_DEBUG((LM_DEBUG, "house::installing reconnect timer %d secs\n",
                   conn_in_secs));
        QTimer::singleShot(conn_in_secs*1000, this, SLOT(reconnect()));
        //}
    }
        break;
    case message::ctz_group_server_unreachable:
    {
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
        _status_tmp = langstr("main_win/ip_detected",  ms->str().c_str());
        self_model()->user().ip_as_string(ms->str());
        _check_if_self_blocked();
        break;
    case message::external_ip_fetch_fail:
        _status_tmp = langstr("main_win/ip_failed",    ms->str().c_str()); break;
    default:
        return false;
    }

    _update_status();
    return true;
}

void
house::handle_global_block_users(::message_block_users *mb) {
    _global_ip_block.handle_message(mb);
    _check_if_self_blocked();
}

void
house::_check_if_self_blocked() {
    if (self_model()->user().ip_as_string().empty() ||
        _global_ip_block.size() == 0)
    {
        return;
    }

    struct in_addr a;
    if (!ACE_OS::inet_aton(self_model()->user().ip_as_string().c_str(), &a)) {
        ACE_ERROR((LM_ERROR, "house::_check_if_self_blocked: "
                   "could not make IP of %s\n",
                   self_model()->user().ip_as_string().c_str()));
        return;
    }

    a.s_addr = ntohl(a.s_addr);

    if (_global_ip_block.is_blocked(a.s_addr)) {
        // Let user know he is in global block list, how unfortunate for him
        // IMPROVE Qt test this
        // IMPROVE Qt there used to be url to RV Pub Forum in here, see below. Not sure if it's really necessary anymore.
        QMessageBox::information(::house_win(), langstr("app/self_on_global_ignore_topic"), langstr("app/self_on_global_ignore"));

//        house_message_box::information(
//            ::app(), FX::MBOX_OK,
//            conf()->get("www", "pub_forum").c_str(),
//            langstr("app/self_on_global_ignore_topic"),
//            langstr("app/self_on_global_ignore")
//        );
    }
}

void
house::_update_status() {
    std::string status;

    STRING_JOIN(status, "; ", _status_ctz);
    STRING_JOIN(status, "; ", _status_dht);
    STRING_JOIN(status, "; ", _status_dht_extra[0]);
    STRING_JOIN(status, "; ", _status_dht_extra[1]);
    STRING_JOIN(status, "; ", _status_tmp);

    this->statusBar()->showMessage(status.c_str());
}

void
house::_room_buttons_status() {
    if (self_model()->joining_room()   != chat_gaming::room::id_type() ||
        self_model()->user().room_id() != chat_gaming::room::id_type())
    {
        _action_create_room->setDisabled(true);
        _action_join_room->setDisabled(true);

    } else {
        _action_create_room->setEnabled(true);
        _action_join_room->setEnabled(true);
    }
}

void
house::interrupt() {
    if (!self_model()->joining_room().empty()) {
        self_model()->joining_room(chat_gaming::room::id_type());
        // If aborted before joined, necessary to do this update so that
        // the recipient won't reserve a slot for the joiner anymore.
        self_model()->user_send();
    }

    interruptable_action_update();
}

void
house::interruptable_action_update() {
    // True if interruptable
    bool intr = false;
    if (!self_model()->joining_room().empty()) {
        intr = true;
    }

    _action_cancel->setEnabled(intr);
    _status_tmp.clear();
    _update_status();
}

void
house::interruptable_action_update(const std::string &s) {
    interruptable_action_update();

    _status_tmp = s;
    _update_status();
}

void
house::refresh() {
    ::message *m = new ::message(::message::refresh);
    net_messenger()->send_msg(m);

    // disable refresh button for 8 seconds
    QTimer::singleShot(8000, this, SLOT(enable_refresh()));
    _action_refresh->setDisabled(true);
}

void
house::enable_refresh() {
    _action_refresh->setEnabled(true);
}

void
house::reconnect () {
    ACE_DEBUG((LM_DEBUG, "house::reconnect received\n"));
    if (_ctz_disconnected) {
        ACE_DEBUG((LM_DEBUG, "house::trying to reconnect\n"));
        net_connect();
    }
}

void
house::dht_disconnected() {
    ACE_DEBUG((LM_DEBUG, "house::on_dht_disconnected\n"));

    if (_last_dht_status_message_id != message::dht_disconnected) {
        ACE_DEBUG((LM_DEBUG, "house: was not disconnected anymore, "
                  "%d/%d\n",
                  _last_dht_status_message_id, message::dht_disconnected));
        /* No more on disconnected state, either retrying or succeeded,
         * so do nothing unless receive disconnected again
         */
        return;
    }

    if (conf()->get<bool>("main/no_router_fw_help", false))
        return;

    if (!_router_fw_help_showed) {
        ACE_DEBUG((LM_DEBUG, "house::on_dht_disconnected:help box\n"));
        // Display a message box
        (new router_fw_help(this))->show_when_possible();
        _router_fw_help_showed = true;
    }
    return;
}

// users_view::observer interface
void
house::user_added(const chat_gaming::user &u) {
    if (app_opts.flash_new_user() &&
        self_model()->user().status() != chat_gaming::user::status_playing)
    {
        os::flash_window(this);
    }
    _chat_view->status_message(langstr("chat/user_entered", u.display_id().c_str()));

    std::string login_lowercase = u.login_id();
    std::transform(login_lowercase.begin(), login_lowercase.end(), login_lowercase.begin(), tolower);
    if (user_conf()->get("users", login_lowercase.c_str(), std::string()) == "block")
    {
        // TODO users.cpp uses the exact same functionality to create addr from string,
        // put into some utility function
        struct in_addr a;
        std::string ipstr = u.ip_as_string();
        if (!ACE_OS::inet_aton(ipstr.c_str(), &a)) {
            ACE_ERROR((LM_ERROR, "house::user_added: IP '%s' not "
                       "recognized\n", ipstr.c_str()));
            return;
        }

        a.s_addr = ntohl(a.s_addr);
        if (a.s_addr == INADDR_ANY      ||
            a.s_addr == INADDR_LOOPBACK ||
            a.s_addr == INADDR_NONE)
        {
            ACE_ERROR((LM_ERROR, "house::user_added: IP '%s' invalid as IP\n",
                       ipstr.c_str()));
            return;
        }

        message_block_users *m = new message_block_users(
            ::message::block_users,
            self_model()->user(),
            self_model()->sequence(),
            0
        );
        m->ip_push_back(a.s_addr);
        net_messenger()->send_msg(m);
    }
}

void
house::user_removed(const chat_gaming::user &u) {
    _chat_view->status_message(langstr("chat/user_exited", u.display_id().c_str()));
}

void
house::user_blocked(const std::string &display_id) {
    _chat_view->status_message(langstr("chat/user_ignored",display_id.c_str()));
}

// rooms_view::observer interface
void
house::room_added(const chat_gaming::room &r) {
    _chat_view->status_message(langstr("chat/room_created", r.topic().c_str()));
    // Don't flash the window, however, if playing or in some room
    // already
    if (app_opts.flash_new_room() &&
        self_model()->user().status() != chat_gaming::user::status_playing &&
        self_model()->user().room_id().empty())
    {
        os::flash_window(this);
    }
}

void
house::room_removed(const chat_gaming::room &r) {
    _chat_view->status_message(langstr("chat/room_closed", r.topic().c_str()));
}

} // ns window
} // ns gui
