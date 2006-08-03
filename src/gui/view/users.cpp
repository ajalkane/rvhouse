#include <sstream>
#include <functional>
#include <iterator>

#include <fx.h>

#include "../../main.h"
#include "../../util.h"
#include "../../exception.h"
#include "../../model/self.h"
#include "../../messaging/messenger.h"
#include "../../messaging/message_user.h"
#include "../../messaging/message_room.h"
#include "../../messaging/message_room_command.h"
#include "../../chat_gaming/user.h"
#include "../../chat_gaming/room.h"
#include "../../multi_feed/util.h"
#include "../../multi_feed/algorithm.h"
#include "../../multi_feed/user_accessors.h"
#include "../../app_options.h"
#include "../../icon_store.h"

#include "../house_app.h"
#include "../util/util.h"
#include "users.h"

namespace gui {
namespace view {

FXDEFMAP(user_item) user_item_map[]={
    FXMAPFUNC(SEL_QUERY_TIP,0,user_item::on_query_tip),
};
FXDEFMAP(users) users_map[]={
    FXMAPFUNC(SEL_QUERY_TIP,         0, users::on_query_tip),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0, users::on_user_rightclick),
    FXMAPFUNC(SEL_COMMAND, users::ID_USER_KICK, users::on_user_kick),
    FXMAPFUNC(SEL_COMMAND, users::ID_SHARE_TRACKS, users::on_share),
    FXMAPFUNC(SEL_COMMAND, users::ID_GET_TRACKS,   users::on_share),
    FXMAPFUNC(SEL_COMMAND, users::ID_PRIV_MSG,     users::on_priv_msg),
    FXMAPFUNC(SEL_DOUBLECLICKED,0,users::on_doubleclick),   
    FXMAPFUNCS(SEL_COMMAND, users::ID_STATUS_CHATTING, 
                            users::ID_STATUS_AWAY,
                            users::on_status_change),
    
    
};

//FXIMPLEMENT(users, FXTreeList, NULL, 0)
//FXIMPLEMENT(user_item, FXTreeItem, NULL, 0)

FXIMPLEMENT(users, FXTreeList, users_map, ARRAYNUMBER(users_map))
FXIMPLEMENT(user_item, component::house_tree_item, user_item_map, ARRAYNUMBER(user_item_map))

const FXColor user_item::col_dht_conn = fxcolorfromname("Cobalt");
const FXColor user_item::col_ctz_conn = fxcolorfromname("Aquamarine4");
const FXColor user_item::col_dht_ctz_conn = fxcolorfromname("DarkBlue");
const FXColor user_item::col_nat_dht_ctz_conn = fxcolorfromname("Brick");
const FXColor user_item::col_not_validated = fxcolorfromname("Red");

user_item::user_item(
) : _dht_conn(false), _ctz_conn(false), _validated(false), _natted(false),
    _display_group(group_chatting)
{
    color(col_dht_conn);
}

long
user_item::on_query_tip(FXObject *sender,FXSelector sel,void *ptr) {
    // ACE_DEBUG((LM_DEBUG, "user_item::on_query_tip called!\n"));
    // sender->handle(this,FXSEL(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE),
    //               (void*)&tmp); // _tip_str);
    _resolve_tip();
    sender->handle(this,FXSEL(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE),
                   (void*)&_tip_str);
    return 1;   
}

bool
user_item::user_remove(const chat_gaming::user::id_type &id, int grp) {
    _user_feed.erase_feed(grp);
    return (_user_feed.feeds_size() ? false : true);
}


bool
user_item::resolve_user_state() {
    bool ret = false;
    
    // Check if the user is in playing state
    // TODO better and
    // more general way needed here... perhaps to be done to multi_feed
    // namespace.
    bool is_playing = false;
    bool is_in_room = false;
    bool is_away    = false;
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
        
        if (ui->status() == chat_gaming::user::status_playing)
            is_playing = true;
        if (ui->status() == chat_gaming::user::status_away)
            is_away = true;
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
        
    if (is_playing) {
        setOpenIcon(app_icons()->get("launch_small"));
        setClosedIcon(app_icons()->get("launch_small"));
        _display_group = group_playing;
    } else if (is_away) {
        setOpenIcon(app_icons()->get("user_away"));
        setClosedIcon(app_icons()->get("user_away"));       
        _display_group = (is_in_room ? group_in_room : group_away);
    } else if (is_in_room) {
        setOpenIcon(app_icons()->get("user_in_room_small"));
        setClosedIcon(app_icons()->get("user_in_room_small"));      
        _display_group = group_in_room;
    } else {
        setOpenIcon(NULL);
        setClosedIcon(NULL);
        _display_group = group_chatting;        
    }
    
    FXColor col = FXRGB(0,0,0);
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
    
    if (_sharing_tracks) {
        extra_icon(app_icons()->get("tracks_share_small"));
    } else {
        extra_icon(NULL);       
    }
    this->color(col);
    
    _resolve_user_id() && (ret = true);
    // _resolve_tip()     && (ret = true);
    
    // At least for now always return true to force an update
    ret = true;
    
    return ret;
}

bool
user_item::_resolve_user_id() {
    // FXString user_id;
    std::string display_id = _user_feed.unambiguous_display_id();

    if (display_id.c_str() != getText()) {
        setText(display_id.c_str());        
        return true;
    }
    return false;
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
                  true_to_string(langstr("users/shares_tracks"))))
                 )
          << _tip(header_non_empty_value(uf, "Note", 
                  user_value_accessor(&user_type::getting_tracks,
                  true_to_string(langstr("users/dloads_tracks"))))
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
        tip_s << _tip(header_value(langstr("words/dht"), langstr("users/dht_not_found")));
    }
    if (_ctz_conn) {
        tip_s << _tip(header_value(langstr("words/ctz"), langstr("words/connected")));
    } else {
        tip_s << _tip(header_value(langstr("words/ctz"), langstr("words/disconnected")));
    }

    if (_natted) {
        tip_s << _tip(header_value(langstr("words/note"), 
                 langstr("users/natted")));
    }

    _tip_str = tip_s.str().c_str();
    _tip_str.trim();
    return true;
}

// Returns as a string the value if all (set) values were the same,
// otherwise empty


users::users(
  FXComposite *c, FXObject *tgt, 
  FXSelector sel, FXuint opts, 
  FXint x, FXint y, FXint w, FXint h)
: FXTreeList(c, tgt, sel, opts, x, y, w, h), _observer(NULL)
{
    ACE_DEBUG((LM_DEBUG, "users opts: %d\n", opts));

    _group_items[user_item::group_chatting] = _parent_item(langstr("status/chatting"));
    _group_items[user_item::group_in_room]  = _parent_item(langstr("status/waiting"));
    _group_items[user_item::group_playing]  = _parent_item(langstr("status/playing"));
    _group_items[user_item::group_away]     = _parent_item(langstr("status/away"));
    
  // Right mouse popup
    _popup = new FXMenuPane(this);
    
    // new FXMenuCommand(popupmenu,"Undo",getApp()->undo,icon,&undolist,FXUndoList::ID_UNDO);
    _popup_user = new FXMenuCommand(_popup, langstr("user_popup/user_name"));
    _popup_user->disable();
        
    new FXMenuSeparator(_popup);

    _popup_status = new FXMenuPane(this);
    
    // (new FXMenuCommand(_popup, "Set status:"))->disable();
    new FXMenuCommand(_popup_status, 
        chat_gaming::user::status_to_string(chat_gaming::user::status_chatting),
        NULL, this, ID_STATUS_CHATTING);
    new FXMenuCommand(_popup_status, 
        chat_gaming::user::status_to_string(chat_gaming::user::status_playing),
        NULL, this, ID_STATUS_PLAYING);
    new FXMenuCommand(_popup_status, 
        chat_gaming::user::status_to_string(chat_gaming::user::status_away),
        NULL, this, ID_STATUS_AWAY);

    _popup_status_cascade = new FXMenuCascade(_popup, langstr("words/status"), NULL, _popup_status);
    _popup_share = new FXMenuCommand(_popup, langstr("user_popup/share_tracks"), NULL, this);
    _popup_priv  = new FXMenuCommand(_popup, langstr("user_popup/private_msg"), NULL, this, ID_PRIV_MSG);
}

void
users::create() {
    super::create();
    
    if (!_room_id.empty()) {
        if (_room_id == self_model()->hosting_room().id()) {
            // Add kick command if host...
            new FXMenuCommand(_popup, langstr("user_popup/kick"), NULL, this, ID_USER_KICK);
        }
        // Add items that are already in the room there
        const model::house::group_desc_type &grp_desc = house_model()->group_desc();
        model::house::group_desc_type::const_iterator gi = grp_desc.begin();
        for (; gi != grp_desc.end(); gi++) {
            _update_users(_room_id, gi->first);
        }
    }   
    
    _popup_status->create();
    _popup->create();   
}

users::~users() {
    delete _popup;
    delete _popup_status;
}

void
users::handle_message(::message *msg) {
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
        // Create a new user, but if its users view to some room, only
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
                // this->updateItem(item);
        }   
    }
}

void
users::_remove_dropped() {
    ACE_DEBUG((LM_DEBUG, "users::_remove_dropped\n"));
    user_item *item = NULL;
    while ((item = _user_item_map.pop_dropped())) {
        ACE_DEBUG((LM_DEBUG, "users::_remove_dropped: item %d\n", item));
        // removeItem deletes the allocated item also.
        _del_user_item(item);
        // super::removeItem(item);
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
    // super::appendItem(_item_players, item);
    
    // model::house::user_iterator ui = house_model()->user_find(u.id(), grp_base);
    //if (ui != house_model()->user_end()) {
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
    tree_item_type *parent = _add_user_item2(dgrp);
    this->appendItem(parent, item); 
}

users::tree_item_type *
users::_add_user_item2(int dgrp) {
    ACE_DEBUG((LM_DEBUG, "users::_add_user_item2: dgrp %d\n", dgrp));
        
    _parent_item &parent = _get_group_item(dgrp);

    ACE_DEBUG((LM_DEBUG, "users::_add_user_item2: dgrp %d children %d\n", 
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
    this->removeItem(item);
    _del_user_item2(dgrp);
}

void
users::_del_user_item2(int dgrp) {
    ACE_DEBUG((LM_DEBUG, "users::_del_user_item2: dgrp %d\n", dgrp));
    _parent_item &parent = _get_group_item(dgrp);
    ACE_DEBUG((LM_DEBUG, "users::_del_user_item2: dgrp %d children %d\n", 
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
        this->updateItem(item);
        return;
    }
    
    // Move the item under new father (no tree operations done)
    // Add is before delete so that the existence of the target
    // tree item is quaranteed
    _add_user_item2(new_dgrp);  
    _parent_item &new_group = _get_group_item(new_dgrp);
    // Do move
    this->moveItem(NULL, new_group.item, item);
    
    // Delete without removing the item (possibly deletes the old group)
    _del_user_item2(old_dgrp);
}

void 
users::_add_group_item(_parent_item &group_item, int dgrp) {
    ACE_DEBUG((LM_DEBUG, "users::_add_group_item dgrp %s/%d\n", 
              group_item.text, dgrp));
    tree_item_type *before = NULL;
    for (int i = dgrp + 1; i < (int)array_sizeof(_group_items); i++) {
        _parent_item &git = _get_group_item(i);
        if (git.visible()) {
            before = git.item;
            break;
        }
    }

    group_item.item = this->insertItem(before, NULL, group_item.text);
    super::expandTree(group_item.item);
    super::disableItem(group_item.item);
}

void
users::_del_group_item(_parent_item &group_item) {
    ACE_DEBUG((LM_DEBUG, "users::_del_group_item '%s'\n", group_item.text));
    this->removeItem(group_item.item);
    group_item.item     = NULL;
    group_item.children = 0;
}

long
users::on_query_tip(FXObject *sender,FXSelector sel,void *ptr) {
    // ACE_DEBUG((LM_DEBUG, "users::on_query_tip called!\n"));

    FXint x,y; FXuint buttons;
    getCursorPosition(x,y,buttons);
    FXTreeItem *item=getItemAt(x,y);
    if (item) { 
        if (item->handle(sender, sel, ptr)) return 1;
        // else ACE_DEBUG((LM_DEBUG, "users::on_query_tip: target did not handle\n"));
    }
    
    return 0;
}

long
users::on_user_rightclick(FXObject *sender,FXSelector sel,void *ptr) {
    ACE_DEBUG((LM_DEBUG, "users::on_user_rightclick\n"));
    FXEvent *e = reinterpret_cast<FXEvent *>(ptr);

    if (e->moved) return 0;
    
    item_type *item = dynamic_cast<item_type *>(getItemAt(e->click_x,e->click_y));

    if (!item) return 0;

    /* User name displayed in the pop up */
    std::string user = item->feed_item().unambiguous_display_id();
    /* multi_feed::value(
        item->feed_item(),
        multi_feed::user_value_accessor(&chat_gaming::user::display_id)
    ); */
    _popup_user->setText(user.c_str());
    
    /* Sharing of tracks */
    bool _share_enable = false;
    bool _priv_enable  = _priv_msg_enable(item);
    if (user == self_model()->user().login_id()) {
        _popup_status_cascade->enable();
        _popup_share->setText(langstr("user_popup/share_tracks"));
        _popup_share->setSelector(ID_SHARE_TRACKS);
        // _popup_share->setSelector(FXSEL(SEL_COMMAND, ID_SHARE_TRACKS));
        _share_enable = (item->sharing_tracks() || item->getting_tracks()
                         ? false : true);
    } else {
        _popup_status_cascade->disable(); // ->show(); // disable();
        _popup_share->setText(langstr("user_popup/dload_tracks"));
        _popup_share->setSelector(ID_GET_TRACKS);
        // _popup_share->setSelector(FXSEL(SEL_COMMAND, ID_GET_TRACKS));
        _share_enable = (item->sharing_tracks() && 
                         !self_model()->user().getting_tracks() &&
                         !self_model()->user().sharing_tracks()
                         ? true : false);
    }
    _share_enable ? _popup_share->enable() : _popup_share->disable();
    _priv_enable  ? _popup_priv->enable()  : _popup_priv->disable();
    
    /* The item is selected so that when the user chooses an action
     * from the popup, it can be targete
     */ 
    selectItem(item);
        
    _popup->popup(NULL, e->root_x, e->root_y);
    getApp()->runModalWhileShown(_popup);   

    ACE_DEBUG((LM_DEBUG, "users::on_user_rightclick away\n"));

    return 0;
}

long
users::on_status_change(FXObject *sender,FXSelector sel,void *ptr) {
    int new_status = 0;
    
    switch (FXSELID(sel)) {
    case ID_STATUS_CHATTING: new_status = chat_gaming::user::status_chatting; break;
    case ID_STATUS_PLAYING : new_status = chat_gaming::user::status_playing;  break;
    case ID_STATUS_AWAY    : new_status = chat_gaming::user::status_away;     break;
    }

    if (new_status && new_status != self_model()->user().status()) {
        self_model()->user().status(new_status);
        self_model()->user_send();      
    }
    
    return 0;
}

long
users::on_user_kick(FXObject *sender,FXSelector sel,void *ptr) {
    ACE_DEBUG((LM_DEBUG, "users::on_user_kick\n"));
        
    item_type *item = selected_item();

    if (!item) return 0;

    std::string user_id = multi_feed::value(
        item->feed_item(),
        multi_feed::user_id_accessor(multi_feed::user_id_to_id_string())
    );

    ACE_DEBUG((LM_DEBUG, "users::on_user_kick: got '%s'\n",
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
    
    return 0;
}

long
users::on_share(FXObject *sender,FXSelector sel,void *ptr) {
    ACE_DEBUG((LM_DEBUG, "users::on_share\n"));
    item_type *item = selected_item();

    if (!item) return 0;

    std::string ip = multi_feed::value(
        item->feed_item(),
        multi_feed::user_value_accessor(&chat_gaming::user::ip_as_string)
    );

    switch (FXSELID(sel)) {
    case ID_SHARE_TRACKS:
        ::app()->launch_rvtm();
        break;
    case ID_GET_TRACKS: 
        ::app()->launch_rvtm(ip);
        break;
    default:
        ACE_ERROR((LM_ERROR, "users::on_share invalid id %d\n", FXSELID(sel)));
    }
    
    return 0;
}

long
users::on_doubleclick(FXObject *sender,FXSelector sel,void *ptr) {
    ACE_DEBUG((LM_DEBUG, "users::on_user_doubleclick\n"));
    FXEvent *e = reinterpret_cast<FXEvent *>(ptr);
    
    item_type *item = dynamic_cast<item_type *>(getItemAt(e->click_x,e->click_y));

    if (!item) return 0;

    /* The item is selected so that the even can be forwarded
     * to priv message
     */ 
    selectItem(item);
    
    return this->on_priv_msg(sender, sel, ptr);
}

long
users::on_priv_msg(FXObject *sender,FXSelector sel,void *ptr) {
    ACE_DEBUG((LM_DEBUG, "users::on_priv_msg\n"));
        
    item_type *item = selected_item();

    if (!item) {
        ACE_DEBUG((LM_DEBUG, "users::on_priv_msg no item found\n"));
        return 0;
    }

    if (!_priv_msg_enable(item)) {
        ACE_DEBUG((LM_DEBUG, "users::on_priv_msg not enabled\n"));
        return 0;
    }
    
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
    
    return 0;
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

} // ns view
} // ns gui
