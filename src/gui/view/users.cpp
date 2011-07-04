#include <sstream>
#include <functional>
#include <iterator>

#include <QtGui>

#include "../window/house.h"
#include "../../app_options.h"
#include "../../main.h"
#include "../../util.h"
#include "../../exception.h"
#include "../../icon_store.h"
#include "../../model/self.h"
#include "../../messaging/messenger.h"
#include "../../messaging/message_block_users.h"
#include "../../messaging/message_user.h"
#include "../../messaging/message_room.h"
#include "../../messaging/message_room_command.h"
#include "../../chat_gaming/user.h"
#include "../../chat_gaming/room.h"
#include "../../multi_feed/util.h"
#include "../../multi_feed/algorithm.h"
#include "../../multi_feed/user_accessors.h"

#include "../house_app.h"
#include "users.h"

namespace {
    const QColor col_dht_conn( 61, 89,171,255); // fxcolorfromname("Cobalt");
    const QColor col_ctz_conn(69,139,116,255); //  fxcolorfromname("Aquamarine4");
    const QColor col_dht_ctz_conn(0,0,139,255); // = fxcolorfromname("DarkBlue");
    const QColor col_nat_dht_ctz_conn(156,102, 31,255); //  = fxcolorfromname("Brick");
    const QColor col_not_validated("red"); // fxcolorfromname("Red");
}

namespace gui {
namespace view {


user_item::user_item(
) : _dht_conn(false), _ctz_conn(false), _validated(false), _natted(false),
    _display_group(group_chatting)
{
}

bool
user_item::user_remove(const chat_gaming::user::id_type &id, int grp) {
    _user_feed.erase_feed(grp);
    return (_user_feed.feeds_size() ? false : true);
}
//
//

bool
user_item::resolve_user_state() {
    ACE_DEBUG((LM_DEBUG, "user_item::_resolve_user_stats\n"));

    bool ret = false;

    // Check if the user is in playing state
    // TODO better and
    // more general way needed here... perhaps to be done to multi_feed
    // namespace.
    bool is_playing = false;
    bool is_in_room = false;
    bool is_away    = false;
    bool is_dnd     = false; // do not disturb

    _dht_conn  = false;
    _ctz_conn  = false;
    _natted    = false;
    _validated = false;
    _sharing_tracks = false;
    _getting_tracks = false;

    const
    model::house::group_desc_type &g = house_model()->group_desc();
    model::house::group_desc_type::const_iterator gi = g.begin();
    for (; gi != g.end(); gi++) {
        multi_feed::user_item::feed_id_map_type::const_iterator uid_i
          = _user_feed.find(gi->first);
        if (uid_i == _user_feed.end()) continue;

        model::house::user_iterator ui =
          house_model()->user_find(uid_i->second, gi->first);
        if (ui == house_model()->user_end()) continue;

        ACE_DEBUG((LM_DEBUG, "user_item::_resolve_user_stats: switch ui->status()\n"));

        switch (ui->status()) {
        case chat_gaming::user::status_chatting: break;
        case chat_gaming::user::status_playing: is_playing = true; break;
        case chat_gaming::user::status_away   : is_away    = true; break;
        case chat_gaming::user::status_dont_disturb: is_dnd = true; break;
        }

        if (ui->room_id() != chat_gaming::room::id_type())
            is_in_room = true;
        if (gi->first == message::dht_group_base) {
            _dht_conn = true;
            if (ui->node() && ui->node()->addr().get_port_number() !=
                NETCOMGRP_SERVERLESS_DEFAULT_PORT) {
                _natted = true;
            }
        }
        if (gi->first == message::ctz_group_base)
            _ctz_conn = true;
        if (ui->authenticated()) _validated = true;
        if (ui->sharing_tracks()) _sharing_tracks = true;
        if (ui->getting_tracks()) _getting_tracks = true;
    }

    ACE_DEBUG((LM_DEBUG, "user_item::_resolve_user_stats iffing status\n"));

    if (is_playing) {
        this->setIcon(column_name, app_icons()->get("launch_small"));
        _display_group = group_playing;
    } else if (is_away) {
        this->setIcon(column_name, app_icons()->get("user_away"));
        _display_group = (is_in_room ? group_in_room : group_away);
    } else if (is_dnd) {
        this->setIcon(column_name, app_icons()->get("user_dont_disturb"));
        _display_group = (is_in_room ? group_in_room : group_dnd);
    } else if (is_in_room) {
        this->setIcon(column_name, app_icons()->get("user_in_room_small"));
        _display_group = group_in_room;
    } else {
        this->setIcon(column_name, QIcon());
        _display_group = group_chatting;
    }

    ACE_DEBUG((LM_DEBUG, "user_item::_resolve_user_stats: resolving user_item color\n"));

    QColor col = QApplication::palette().color(QPalette::Foreground);
    if (!_validated) {
        col = col_not_validated;
    } else if (_dht_conn && _ctz_conn) {
        if (_natted) {
            col = col_nat_dht_ctz_conn;
        } else {
            col = col_dht_ctz_conn;
        }
    } else if (_dht_conn) {
        col = col_dht_conn;
    } else if (_ctz_conn) {
        col = col_ctz_conn;
    }

    ACE_DEBUG((LM_DEBUG, "user_item::_resolve_user_stats: setting color\n"));

    this->setForeground(column_name, col);

    ACE_DEBUG((LM_DEBUG, "user_item::_resolve_user_stats: resolving user id\n"));

    _resolve_user_id() && (ret = true);

    return true;
}

bool
user_item::_resolve_user_id() {
    // FXString user_id;
    std::string display_id = _user_feed.unambiguous_display_id();

    if (text(column_name) != display_id.c_str()) {
        this->setText(column_name, display_id.c_str());
        return true;
    }
    return false;
}

bool
user_item::update_tip() {
    _resolve_tip();
    return true;
}

bool
user_item::_resolve_tip() {
    std::ostringstream tip_s;

    using multi_feed::header_value;
    using multi_feed::header_non_empty_value;
    using multi_feed::user_accessor;
    using multi_feed::user_value_accessor;
    using multi_feed::user_room_id_accessor;
    using multi_feed::true_to_string;
    using multi_feed::validation_to_string;
    using multi_feed::room_id_to_string;
    using multi_feed::user_to_status_string;
    const multi_feed::user_item &uf = _user_feed; // Just a shorter alias

    tip_s << _tip(header_value(uf, langstr("words/user_id"),
                  user_value_accessor(&user_type::login_id)))
          << _tip(header_non_empty_value(uf, langstr("words/warning"),
                  user_value_accessor(&user_type::validation,
                                      validation_to_string()))
                 )
          << _tip(header_value(uf, langstr("words/status"),
                  user_accessor(user_to_status_string()))
                 )
          << _tip(header_non_empty_value(uf, langstr("words/room"),
                  user_room_id_accessor(
                      room_id_to_string()))
                 )
          << _tip(header_non_empty_value(uf, langstr("words/note"),
                  user_value_accessor(&user_type::sharing_tracks,
                  true_to_string(langstr("users_view/shares_tracks"))))
                 )
          << _tip(header_non_empty_value(uf, "Note",
                  user_value_accessor(&user_type::getting_tracks,
                  true_to_string(langstr("users_view/dloads_tracks"))))
                 );

if (app_opts.debug()) {
    tip_s << _tip(header_value(uf, "Room ID",
                  user_room_id_accessor(multi_feed::dummy_string_to_string()))
                 )

          << _tip(header_value(uf, "IP",
                  user_value_accessor(&user_type::ip_as_string))
                 );
}

    if (_dht_conn) {
        tip_s << _tip(header_value(langstr("words/dht"), langstr("words/connected")));
    } else {
        tip_s << _tip(header_value(langstr("words/dht"), langstr("users_view/dht_not_found")));
    }
    if (_ctz_conn) {
        tip_s << _tip(header_value(langstr("words/ctz"), langstr("words/connected")));
    } else {
        tip_s << _tip(header_value(langstr("words/ctz"), langstr("words/disconnected")));
    }

    if (_natted) {
        tip_s << _tip(header_value(langstr("words/note"),
                 langstr("users_view/natted")));
    }

    _tip_str = tip_s.str().c_str();
    _tip_str = _tip_str.trimmed();
    return true;
}

// Returns as a string the value if all (set) values were the same,
// otherwise empty


users::users(QWidget *parent, const chat_gaming::room::id_type &rid)
: QTreeWidget(parent), _observer(NULL), _room_id(rid)
{
    this->setColumnCount(1);
    this->header()->hide();

    _group_items[user_item::group_chatting] = _parent_item(langstr("status/chatting"));
    _group_items[user_item::group_in_room]  = _parent_item(langstr("status/waiting"));
    _group_items[user_item::group_playing]  = _parent_item(langstr("status/playing"));
    _group_items[user_item::group_away]     = _parent_item(langstr("status/away"));
    _group_items[user_item::group_dnd]      = _parent_item(langstr("status/dont_disturb"));

    _create_actions();
    _create_context_menu();
    _create_signals();

    this->installEventFilter(this);

    if (!_room_id.empty()) {
        // Add items that are already in the room there
        const model::house::group_desc_type &grp_desc = house_model()->group_desc();
        model::house::group_desc_type::const_iterator gi = grp_desc.begin();
        for (; gi != grp_desc.end(); gi++) {
            _update_users(_room_id, gi->first);
        }
    }
}


void
users::_create_actions() {
    // IMPROVE: actions common with window::house should be created and handled in house_app for example.
    // IMPROVE: seems like there is overlap in string names, there is menu_player/chatting and chat_gaming::user::status_to_string.
    //      Is this necessary or should they be unified under one key?
    _action_user = new QAction(langstr("user_popup/user_name"), this);
    _action_user->setDisabled(true);

    _action_chatting = new QAction(chat_gaming::user::status_to_string(chat_gaming::user::status_chatting), this);
    _action_chatting->setCheckable(true);
    _action_playing = new QAction(chat_gaming::user::status_to_string(chat_gaming::user::status_playing), this);
    _action_playing->setCheckable(true);
    _action_away = new QAction(chat_gaming::user::status_to_string(chat_gaming::user::status_away), this);
    _action_away->setCheckable(true);
    _action_donotdisturb = new QAction(chat_gaming::user::status_to_string(chat_gaming::user::status_dont_disturb), this);
    _action_donotdisturb->setCheckable(true);
    _action_chatting->setChecked(true);

    _action_block           = new QAction(langstr("user_popup/ignore_user"), this);
    _action_private_message = new QAction(langstr("user_popup/private_msg"), this);
    _action_kick            = new QAction(langstr("user_popup/kick"), this);

}

void
users::_create_context_menu() {
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    _context_menu = new QMenu(this);

    _context_menu->addAction(_action_user);
    _context_menu->addSeparator();
    _status_submenu = new QMenu(langstr("words/status"), _context_menu);
    _action_statusgroup = new QActionGroup(_status_submenu);
    _action_statusgroup->addAction(_action_chatting);
    _action_statusgroup->addAction(_action_playing);
    _action_statusgroup->addAction(_action_away);
    _action_statusgroup->addAction(_action_donotdisturb);

    _status_submenu->addActions(_action_statusgroup->actions());
    _context_menu->addMenu(_status_submenu);

    _context_menu->addAction(_action_private_message);
    _context_menu->addAction(_action_block);

    if (!_room_id.empty()) {
        if (_room_id == self_model()->hosting_room().id()) {
            _context_menu->addAction(_action_kick);
        }
    }

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(on_context_menu(const QPoint&)));
}

void
users::_create_signals() {
    connect(_action_chatting,     SIGNAL(triggered()), house_win(), SLOT(change_user_status_to_chatting()));
    connect(_action_away,         SIGNAL(triggered()), house_win(), SLOT(change_user_status_to_away()));
    connect(_action_playing,      SIGNAL(triggered()), house_win(), SLOT(change_user_status_to_playing()));
    connect(_action_donotdisturb, SIGNAL(triggered()), house_win(), SLOT(change_user_status_to_dont_disturb()));

    connect(_action_block,           SIGNAL(triggered()), this, SLOT(block_selected_user()));
    connect(_action_private_message, SIGNAL(triggered()), this, SLOT(open_private_msg_to_selected_user()));
    connect(_action_kick,            SIGNAL(triggered()), this, SLOT(kick_selected_user()));

    connect(this,       SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(toggle_selection(QTreeWidgetItem *, int)));


}

users::~users() {
}

void
users::handle_message(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "users::handle_message: id=%d\n", msg->id()));

    switch (msg->id()) {
    case message::user:
    {
        message_user *u = dynamic_ptr_cast<message_user>(msg);
        _update_user(u->user().id(), u->group_base());
        _remove_dropped();
    }
        break;
    case message::user_left:
    {
        message_user *u = dynamic_ptr_cast<message_user>(msg);
        _remove_user(u->user(), u->group_base());
        _remove_dropped();
    }
        break;
    case message::room:
    {
        message_room *r = dynamic_ptr_cast<message_room>(msg);
        _room_update(r->room().id(), r->group_base());
    }
        break;
    }
}

void
users::_update_user(const chat_gaming::user::id_type &uid, int grp_base) {
    model::house::house_type::user_iterator ui =
      house_model()->user_find(uid, grp_base);
      
    if (ui != house_model()->user_end()) {
        _update_user(*ui, grp_base);
    }
}

void
users::_update_user(const chat_gaming::user &u, int grp_base) {
    ACE_DEBUG((LM_DEBUG, "users::_update_user for user/group %s/%d, node:%d\n",
              u.id().c_str(), grp_base, u.id().node()));

    item_type *item = _user_item_map.find(u.id()); // _find_item(u.id());
    
    if (item == NULL) {
        // Create a new user, but if its user's view to some room, only
        // if the to be user is in the room
        
        if (_room_id.empty() || _room_id == u.room_id())
            _new_user(u, grp_base);
    } else {
        ACE_DEBUG((LM_DEBUG, "users::_update_user found user item\n"));
        bool sharing_tracks = item->sharing_tracks();
        bool getting_tracks = item->getting_tracks();       
        // Check if this is a view to users in a room, that if the user is
        // no more in the room, then remove it
        if (_room_id.empty() || _room_id == u.room_id()) {
            item->user(u.id(), grp_base);
            _user_item_map.resolve(u.id());
            int old_dgrp = item->display_group();
            if (item->resolve_user_state())
                _update_user_item(item, old_dgrp);
                
            if (sharing_tracks != item->sharing_tracks() && _observer)
                _observer->sharing_tracks_changed(u, item->sharing_tracks());
            if (getting_tracks != item->getting_tracks() && _observer)
                _observer->getting_tracks_changed(u, item->getting_tracks());
        } else {
            _remove_user(u, grp_base);
        }           
        ACE_DEBUG((LM_DEBUG, "1\n"));
    }
}

void
users::_update_users(const chat_gaming::room::id_type &rid, int grp_base) {
    // Updates user infos that are in the specified room.
    ACE_DEBUG((LM_DEBUG, "users::_update_users in room %s/%d\n",
              rid.c_str(), grp_base));
              
    typedef std::list<model::house::user_type::id_type> ins_type;
    ins_type users_in_room;
    std::insert_iterator<ins_type> 
      i_itr(users_in_room, users_in_room.begin());

    house_model()->user_ids_in_room(rid, i_itr, grp_base);
    
    ins_type::iterator i = users_in_room.begin();
    for (; i != users_in_room.end(); i++) {
        _update_user(*i, grp_base);
    }
}

void
users::_room_update(const chat_gaming::room::id_type &rid, int grp_base)
{
    ACE_DEBUG((LM_DEBUG, "users::_room_update for room %s/%d\n",
              rid.c_str(), grp_base));
    
    model::house::house_type::room_iterator ric, rip;
    model::house *hmc, *hmp;
    hmc = house_model();
    hmp = house_model_prev();
    ric = hmc->room_find(rid, grp_base);
    rip = hmp->room_find(rid, grp_base);
    
    // If the room topic has changed, must update user information
    // for all the users in the view.
    if (ric != hmc->room_end() &&
        rip != hmp->room_end())
    {
        // If no change has happened to the room status, do nothing more
        if (ric->topic() == rip->topic()) {
            ACE_DEBUG((LM_DEBUG, "users::_room_update no change in topic\n"));
            return;
        }
        // Do necessary updates to the users that are in this room.
        _update_users(rid, grp_base);
        return;
    }
    
    // If gotten this far either uic or uip is no more and thus update
    // must be done
    _update_users(rid, grp_base);
}

void
users::_remove_user(const chat_gaming::user &u, int grp_base) {
    ACE_DEBUG((LM_DEBUG, "users::_remove_user for user/group %s/%d\n",
              u.id().c_str(), grp_base));

    user_item *item = _user_item_map.find(u.id());
    
    if (item != NULL) {
        ACE_DEBUG((LM_DEBUG, "users::_remove_user: removing\n"));
        
        if (item->user_remove(u.id(), grp_base)) {
            ACE_DEBUG((LM_DEBUG, "users::_remove_user: removing from tree\n"));
            _user_item_map.erase(u.id());
            if (!u.login_id().empty()) {
                if (_observer) _observer->user_removed(u);
            }
        } else {
            _user_item_map.resolve(u.id());
            int old_dgrp = item->display_group();
            if (item->resolve_user_state())
                this->_update_user_item(item, old_dgrp);
        }   
    }
}

void
users::_remove_dropped() {
    ACE_DEBUG((LM_DEBUG, "users::_remove_dropped\n"));
    user_item *item = NULL;
    while ((item = _user_item_map.pop_dropped())) {
        ACE_DEBUG((LM_DEBUG, "users::_remove_dropped: item %d\n", item));
        _del_user_item(item);
    }
}

void
users::_new_user(const chat_gaming::user &u, int grp_base) {
    ACE_DEBUG((LM_DEBUG, "users::_new_user for user/group %s/%d\n",
              u.id().c_str(), grp_base));

    item_type *item = new item_type();
    _user_item_map.insert(u.id(), item);
    _user_item_map.resolve(u.id());
    
    item->user(u.id(), grp_base);

    ACE_DEBUG((LM_DEBUG, "users::_new_user: resolve\n"));
    item->resolve_user_state();
    ACE_DEBUG((LM_DEBUG, "users::_new_user: add_user_item\n"));
    _add_user_item(item);
    ACE_DEBUG((LM_DEBUG, "users::_new_user: return\n"));
    
    if (!u.login_id().empty()) {
        ACE_DEBUG((LM_DEBUG, "users::_new_user calling user added for %s\n",
                  u.login_id().c_str()));
        if (_observer) _observer->user_added(u); // *ui);
    }
}

void
users::_add_user_item(user_item *item, int dgrp) {
    ACE_DEBUG((LM_DEBUG, "users::_add_user_item: dgrp %d\n", dgrp));
    if (dgrp == -1) dgrp = item->display_group();
    tree_item_type *parent = _reserve_place_for_item_in_group(dgrp);
    _add_user_to_group(parent, item);
}

void
users::_add_user_to_group(tree_item_type *parent, item_type *item) {
    parent->addChild(item);
    item->setDisabled(false);
}

users::tree_item_type *
users::_reserve_place_for_item_in_group(int dgrp) {
    ACE_DEBUG((LM_DEBUG, "users::_reserve_place_for_item_in_group: dgrp %d\n", dgrp));
        
    _parent_item &parent = _get_group_item(dgrp);

    ACE_DEBUG((LM_DEBUG, "users::_reserve_place_for_item_in_group: dgrp %d children %d\n",
               dgrp, parent.children));
    if (!parent.item) {
        _add_group_item(parent, dgrp);
    }
    parent.children++;
    return parent.item;
}

void
users::_del_user_item(user_item *item, int dgrp) {
    ACE_DEBUG((LM_DEBUG, "users::_del_user_item: dgrp %d\n", dgrp));
    if (dgrp == -1) dgrp = item->display_group();   
    this->removeItemWidget(item, 0);
    delete item;
    _remove_place_for_item_in_group(dgrp);
}

void
users::_remove_place_for_item_in_group(int dgrp) {
    ACE_DEBUG((LM_DEBUG, "users::_remove_place_for_item_in_group: dgrp %d\n", dgrp));
    _parent_item &parent = _get_group_item(dgrp);
    ACE_DEBUG((LM_DEBUG, "users::_remove_place_for_item_in_group: dgrp %d children %d\n",
               dgrp, parent.children));
    if (--parent.children <= 0) {
        _del_group_item(parent);
    }
}

void
users::_update_user_item(user_item *item, int old_dgrp) {
    ACE_DEBUG((LM_DEBUG, "users::_update_user_item: old_dgrp %d\n", old_dgrp));
    int new_dgrp = item->display_group();
    if (new_dgrp == old_dgrp) {
        return;
    }
    
    // Move the item under new father (no tree operations done)
    // Add is before delete so that the existence of the target
    // tree item is quaranteed
    _reserve_place_for_item_in_group(new_dgrp);
    _parent_item &old_group = _get_group_item(old_dgrp);
    _parent_item &new_group = _get_group_item(new_dgrp);
    // Do move
    old_group.item->removeChild(item);
    _add_user_to_group(new_group.item, item);
    
    // Delete without removing the item (possibly deletes the old group)
    _remove_place_for_item_in_group(old_dgrp);
}

void 
users::_add_group_item(_parent_item &group_item, int dgrp) {
    ACE_DEBUG((LM_DEBUG, "users::_add_group_item dgrp %s/%d\n",
              group_item.text, dgrp));

    tree_item_type *item = new tree_item_type(this);
    item->setText(0, group_item.text);
    this->insertTopLevelItem(dgrp, item);

    group_item.item = item;
   item->setExpanded(true);
}

void
users::_del_group_item(_parent_item &group_item) {
    ACE_DEBUG((LM_DEBUG, "users::_del_group_item '%s'\n", group_item.text));
    this->removeItemWidget(group_item.item, 0);
    delete group_item.item;
    group_item.item     = NULL;
    group_item.children = 0;
}

bool
users::eventFilter(QObject *obj, QEvent *event) {
    //ACE_DEBUG((LM_DEBUG, "users::eventFilter: obj %d, event type %d\n", obj, event->type()));
    if (event->type() == QEvent::ToolTip || event->type() == QEvent::WhatsThis) {
        QHelpEvent *help_event = dynamic_cast<QHelpEvent *>(event);
        // ACE_DEBUG((LM_DEBUG, "users::eventFilter: toolTip x %d y %d\n", help_event->x(), help_event->y()));
        // NOTE: if headers used, the position is not correct for itemAt,
        // see QTBUG-16638 http://bugreports.qt.nokia.com/browse/QTBUG-16638
        QPoint viewport_pos = this->viewport()->mapFromGlobal(help_event->globalPos());
        item_type *item = dynamic_cast<item_type *>(this->itemAt(viewport_pos));
        // Returns null if item is not of type item_type (basically is a group item)
        if (item != NULL) {
            // ACE_DEBUG((LM_DEBUG, "users::eventFilter: toolTip updating item\n", help_event->x(), help_event->y()));
            item->update_tip();
            QToolTip::showText(help_event->globalPos(), item->get_tip());
            return true;
        }
    }
    return super::eventFilter(obj, event);
}

void
users::on_context_menu(const QPoint &pos) {
    QPoint globalPos = this->viewport()->mapToGlobal(pos);

    item_type *item = dynamic_cast<item_type *>(this->itemAt(pos));

    if (!item) return;

    /* User name displayed in the pop up */
    std::string user = item->feed_item().unambiguous_display_id();
    _action_user->setText(user.c_str());

    /* Sharing of tracks */
    bool priv_enable  = _priv_msg_enable(item);
    bool block_enable = true;

    ACE_DEBUG((LM_DEBUG, "users::on_context_menu: user '%s', self_model user '%s'\n",
            user.c_str(), self_model()->user().login_id().c_str()));
    if (user == self_model()->user().login_id()) {
        _status_submenu->setEnabled(true);
        block_enable = false;
    } else {
        _status_submenu->setEnabled(false);
    }
    _action_private_message->setEnabled(priv_enable);
    _action_block->setEnabled(block_enable);

    /* The item is selected so that when the user chooses an action
     * from the popup, it can be targeted
     */
    this->setCurrentItem(item);
    _context_menu->popup(globalPos);

    ACE_DEBUG((LM_DEBUG, "users::on_context_menu: away\n"));

}

void
users::kick_selected_user() {
    ACE_DEBUG((LM_DEBUG, "users::kick_selected_user\n"));

    item_type *item = selected_item();

    if (!item) return;

    std::string user_id = multi_feed::value(
        item->feed_item(),
        multi_feed::user_id_accessor(multi_feed::user_id_to_id_string())
    );

    ACE_DEBUG((LM_DEBUG, "users::kick_selected_user: got '%s'\n",
              user_id.c_str()));

    if (!user_id.empty()) {
        net_messenger()->send_msg(
          new message_room_command(
            ::message::room_kick,
            _room_id,
            chat_gaming::user::id_type(user_id),
            self_model()->user().id(),
            self_model()->sequence(),
            0
          )
        );
    }
}

void
users::block_selected_user() {
    ACE_DEBUG((LM_DEBUG, "users::block_selected_user\n"));

    item_type *item = selected_item();

    if (!item) return;

    std::string ipstr = multi_feed::uniq_value(
        item->feed_item(),
        multi_feed::user_value_accessor(&chat_gaming::user::ip_as_string)
    );

    ACE_DEBUG((LM_DEBUG, "users::on_user_block: got IP '%s'\n",
              ipstr.c_str()));

    // Check, just to make sure, that it really looks like an IP
    // before converting to IP number
    // if (regexp::match("([0-9]{1,3}[.]){3}[0-9]{1,3}", ipstr) {
    struct in_addr a;
    if (!ACE_OS::inet_aton(ipstr.c_str(), &a)) {
        ACE_ERROR((LM_ERROR, "users::on_user_block: IP '%s' not "
                   "recognized\n", ipstr.c_str()));
        return;
    }

    a.s_addr = ntohl(a.s_addr);
    if (a.s_addr == INADDR_ANY      ||
        a.s_addr == INADDR_LOOPBACK ||
        a.s_addr == INADDR_NONE)
    {
        ACE_ERROR((LM_ERROR, "users::on_user_block: IP '%s' invalid as IP\n",
                   ipstr.c_str()));
        return;
    }

    ACE_DEBUG((LM_DEBUG, "users::on_user_block: recognized IP\n"));
    message_block_users *m = new message_block_users(
        ::message::block_users,
        self_model()->user(),
        self_model()->sequence(),
        0
    );
    m->ip_push_back(a.s_addr);
    net_messenger()->send_msg(m);

    if (_observer)
        _observer->user_blocked(item->feed_item().unambiguous_display_id());

    return;
}

void
users::open_private_msg_to_selected_user() {
    ACE_DEBUG((LM_DEBUG, "users::block_selected_user\n"));

    item_type *item = selected_item();

    if (!item) return;

    house_win()->open_private_room(item, 0);
}

bool
users::_priv_msg_enable(const item_type *item) const {
    std::string display_id = item->feed_item().unambiguous_display_id();
    if (display_id != self_model()->user().login_id()) {
        if (item->display_group() != item_type::group_playing)
            return true;
    }

    return false;
}

// IMPROVE: ugly hack so that selected items can be deselected by pressing on header
// Optimally this would work like this:
// - If user is selected and single clicked on it, it would be deselected.
// - If clicked outside of tree area, user is deselected.
void
users::toggle_selection(QTreeWidgetItem *widget_item, int column) {
    ACE_DEBUG((LM_DEBUG, "users::toggle_user\n"));

    item_type *item = dynamic_cast<item_type *>(widget_item);

    if (!item && widget_item) {
        ACE_DEBUG((LM_DEBUG, "users::toggle_user not a room item, clearing selection\n"));
        widget_item->treeWidget()->clearSelection();
        return;
    }

    // item->setSelected(!item->isSelected());
}

} // ns view
} // ns gui
