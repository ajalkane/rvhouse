#include <sstream>
#include <functional>

#include <QtGui>

#include "../../main.h"
#include "../../model/house.h"
#include "../../model/self.h"
#include "../../gui/house_app.h"
#include "../../messaging/messenger.h"
#include "../../messaging/message_user.h"
#include "../../messaging/message_room.h"
#include "../../util.h"
#include "../../exception.h"
#include "../../icon_store.h"
#include "../../chat_gaming/user.h"
#include "../../multi_feed/util.h"
#include "../../multi_feed/algorithm.h"
#include "../../multi_feed/room_accessors.h"
#include "../../multi_feed/room_util.h"

#include "rooms.h"

namespace gui {
namespace view {

room_item::room_item(
    const chat_gaming::room::id_type &rid
) : _room_id(rid), _tip_str("<Empty>")
{
}

bool
room_item::room_remove(const chat_gaming::room::id_type &id, int grp) {
    _room_feed.erase_feed(grp);
    ACE_DEBUG((LM_DEBUG, "room_item::room_remove grp %d, num feeds left %d\n",
              grp, _room_feed.feeds_size()));
    return (_room_feed.feeds_size() ? false : true);
}


void
room_item::resolve_room_state() {
    _resolve_columns();
}

bool
room_item::update_tip() {
    _resolve_tip();
    return true;
}

bool
room_item::_resolve_tip() {
    std::ostringstream str;

    using multi_feed::value;
    using multi_feed::room_value_accessor;
    using multi_feed::user_id_to_host_string;
    using multi_feed::bool_to_yes_no_string;
    using multi_feed::has_password_to_string;
    using multi_feed::streamable_to_string;
    using multi_feed::players_slash_max_by_room_id_fobj;
    const multi_feed::room_item &rf = _room_feed; // Just a shorter alias
    
    str << _tip(header_value(rf, langstr("words/topic"), 
                room_value_accessor(&room_type::topic))
           )
        << _tip(header_value(rf, langstr("words/host"),  
                room_owner_id_accessor(user_id_to_host_string()))
           )
        << _tip(header_non_empty_value(rf, langstr("words/note"), 
                room_value_accessor(&room_type::has_password, 
                                    has_password_to_string()))
           )           
        << _tip(header_value(rf, langstr("words/players"), 
                players_slash_max_by_room_id_fobj())
           )
        << _tip(header_value(rf, langstr("words/laps"), 
                room_value_accessor(&room_type::laps, 
                                    streamable_to_string<size_t>()))
           )
        << _tip(header_value(rf, langstr("words/pickups"), 
                room_value_accessor(&room_type::pickups, 
                bool_to_yes_no_string()))
           )
        << _tip_participants();
        

    _tip_str = str.str().c_str();
    _tip_str = _tip_str.trimmed();
    
    return true;
}

std::string
room_item::_tip_participants() {
    typedef std::list<std::string> users_list_type;
    users_list_type users_list;
    std::insert_iterator<users_list_type> itr(users_list, users_list.end());
    
    multi_feed::users_in_room(_room_id, itr);
    
    std::ostringstream str; // _tip(" ")); 
    str << _tip(" "); // Empty line before participants
    users_list_type::iterator i = users_list.begin();
    for (; i != users_list.end(); i++) {
        str << _tip(multi_feed::user_id_to_string()(*i));
    }
    
    return str.str();
}

bool
room_item::_resolve_columns() {
    using multi_feed::value;
    using multi_feed::room_value_accessor;
    using multi_feed::user_id_to_string;
    using multi_feed::bool_to_yes_no_string;
    using multi_feed::streamable_to_string;
    using multi_feed::players_slash_max_by_room_id_fobj;
    const multi_feed::room_item &rf = _room_feed; // Just a shorter alias
    
    setText(rooms::column_topic,   value(rf, room_value_accessor(&room_type::topic)).c_str());
    setText(rooms::column_host,    value(rf, room_owner_id_accessor(user_id_to_string())).c_str());
    setText(rooms::column_players, value(rf, players_slash_max_by_room_id_fobj()).c_str());
    setText(rooms::column_laps,    value(rf, room_value_accessor(&room_type::laps,
                                                                    streamable_to_string<size_t>())).c_str());
    setText(rooms::column_pickups, value(rf, room_value_accessor(&room_type::pickups,
                                                                    bool_to_yes_no_string())).c_str());

    // Check if password is on
    multi_feed::feed_value_map_type vm;
    std::string pass = 
      _room_feed.values(
        vm, 
        room_value_accessor(&room_type::has_password,
                            bool_to_yes_no_string())
      );
      
    ACE_DEBUG((LM_DEBUG, "rooms_item::_resolve_columns: "
              "has pass: %s, vm size: %d\n", pass.c_str(), vm.size()));
    // If no password in any feed set, display no locked symbol... otherwise
    // if any feed has password display locked icon.
    if (pass == langstr("words/no") || vm.size() == 0) {
        setIcon(0, QIcon());
    } else {
        setIcon(0, app_icons()->get("locked_small"));
    }
    
    return true;
}

rooms::rooms(QWidget *parent)
  : super(parent), _observer(NULL)
{
    this->setColumnCount(5);
    this->setRootIsDecorated(false);
    this->installEventFilter(this);

    _create_header();
}

void
rooms::_create_header() {
    QStringList headers;
    headers.append(langstr("words/topic"));
    headers.append(langstr("words/host"));
    headers.append(langstr("words/players"));
    headers.append(langstr("words/laps"));
    headers.append(langstr("words/pickups"));

    this->setHeaderLabels(headers);

    this->setColumnWidth(0, 200);
    this->setColumnWidth(1, 100);
//    this->setColumnWidth(2, 50);
//    this->setColumnWidth(3, 35);
//    this->setColumnWidth(4, 50);

    // These are small columns so size them according to how much the language requires
    this->resizeColumnToContents(2);
    this->resizeColumnToContents(3);
    this->resizeColumnToContents(4);
}

void
rooms::handle_message(::message *msg) {
    switch (msg->id()) {
    case message::user:
    case message::user_left:
        _handle_user_update(msg);
        _remove_dropped();
        break;
    case message::room:
    {
        message_room *u = dynamic_ptr_cast<message_room>(msg);
        _update_room(u->room(), u->group_base());
        _remove_dropped();
    }
        break;
    case message::room_remove:
    {
        message_room *u = dynamic_ptr_cast<message_room>(msg);
        _remove_room(u->room(), u->group_base());
        _remove_dropped();
    }
        break;
    }
}

void
rooms::_handle_user_update(::message *msg) {
    message_user *m = dynamic_ptr_cast<message_user>(msg);
    
    model::house::house_type::user_iterator uic, uip;
    model::house *hmc, *hmp;
    hmc = house_model();
    hmp = house_model_prev();
    uic = hmc->user_find(m->user().id(), m->group_base());
    uip = hmp->user_find(m->user().id(), m->group_base());
    
    // If the user information has changed do necessary calculations
    // in the room he was in, and the room he is in now
    if (uic != hmc->user_end() &&
        uip != hmp->user_end())
    {
        // If no change has happened to the room status, do nothing more
        if (uic->room_id() == uip->room_id() &&
            uic->status()  == uip->status()) return;
        // Do necessary updates to the rooms.
        _update_room(uip->room_id(), m->group_base());
        _update_room(uic->room_id(), m->group_base());
        return;
    }
    
    // If gotten this far either uic or uip is no more
    if (uip != hmp->user_end()) _update_room(uip->room_id(), m->group_base());
    if (uic != hmc->user_end()) _update_room(uic->room_id(), m->group_base());
}

void
rooms::_update_room(const chat_gaming::room::id_type &rid, int grp_base) {
    model::house::house_type::room_iterator ri =
      house_model()->room_find(rid, grp_base);
      
    if (ri != house_model()->room_end()) {
        _update_room(*ri, grp_base);
    }
}

void
rooms::_update_room(const chat_gaming::room &r, int grp_base) {
    ACE_DEBUG((LM_DEBUG, "rooms::_update_room for room/group %s/%d\n",
              r.id().c_str(), grp_base));

    item_type *item = _room_item_map.find(r.id());
    
    if (item == NULL) {
        // Create a new room
        // But only if there is at least someone in the room!
        if (house_model()->room_size(r.id()) > 0)
            _new_room(r, grp_base);
    } else {
        ACE_DEBUG((LM_DEBUG, "rooms::_update_room found room item\n"));
        item->room(r.id(), grp_base);
        _room_item_map.resolve(r.id());
        
        _update_room_state(item);
    }
}

void
rooms::_update_room_state(item_type *item) {
    item->resolve_room_state();

    // Check if the room should be disabled (if the host
    // is playing then so should be done)
    std::string status = multi_feed::value(
        item->feed_item(),
        multi_feed::room_owner_id_accessor(multi_feed::user_id_to_status_string())
    );
    
    int disable_status = chat_gaming::user::status_playing;
    bool override_disable = pref()->get<bool>("advanced", "allow_started_race_join", false);
    if (!override_disable && status == chat_gaming::user::status_to_string(disable_status)) {
        item->setDisabled(true);
    } else {
        item->setDisabled(false);
    }   
}

void
rooms::_remove_room(const chat_gaming::room &r, int grp_base) {
    ACE_DEBUG((LM_DEBUG, "rooms::_remove_room for room/group %s/%d\n",
              r.id().c_str(), grp_base));

    room_item *item = _room_item_map.find(r.id());
    
    if (item != NULL) {
        ACE_DEBUG((LM_DEBUG, "rooms::_remove_room: removing\n"));
        
        if (item->room_remove(r.id(), grp_base)) {
            ACE_DEBUG((LM_DEBUG, "rooms::_remove_room: removing from tree\n"));
            _room_item_map.erase(r.id());
            if (_observer) _observer->room_removed(r);
        } else {
            _room_item_map.resolve(r.id());
            item->resolve_room_state();
        }   
    }
}

void
rooms::_remove_dropped() {
    ACE_DEBUG((LM_DEBUG, "rooms::_remove_dropped\n"));
    room_item *item = NULL;
    while ((item = _room_item_map.pop_dropped())) {
        ACE_DEBUG((LM_DEBUG, "rooms::_remove_dropped: item %d\n", item));
        this->removeItemWidget(item, 0);
        delete item;
    }
}

void
rooms::_new_room(const chat_gaming::room &r, int grp_base) {
    ACE_DEBUG((LM_DEBUG, "rooms::_new_user for user/group %s/%d\n",
              r.id().c_str(), grp_base));

    item_type *item = new item_type(r.id());
    _room_item_map.insert(r.id(), item);
    _room_item_map.resolve(r.id());
    
    item->room(r.id(), grp_base);
    
    super::addTopLevelItem(item);

    ACE_DEBUG((LM_DEBUG, "rooms::_new_user: resolve\n"));
    _update_room_state(item);
    
    if (_observer) _observer->room_added(r);    
}

bool
rooms::eventFilter(QObject *obj, QEvent *event) {
    // ACE_DEBUG((LM_DEBUG, "rooms::eventFilter: obj %d, event type %d\n", obj, event->type()));
    if (event->type() == QEvent::ToolTip || event->type() == QEvent::WhatsThis) {
        QHelpEvent *help_event = static_cast<QHelpEvent *>(event);
        //ACE_DEBUG((LM_DEBUG, "rooms::eventFilter: toolTip x %d y %d\n", help_event->x(), help_event->y()));
        // NOTE: if headers used, the position is not correct for itemAt,
        // see QTBUG-16638 http://bugreports.qt.nokia.com/browse/QTBUG-16638
        QPoint viewport_pos = this->viewport()->mapFromGlobal(help_event->globalPos());

        QTreeWidgetItem *item_raw = this->itemAt(viewport_pos);
        item_type *item = static_cast<item_type *>(this->itemAt(viewport_pos));
        //ACE_DEBUG((LM_DEBUG, "rooms::eventFilter: item_raw %d, item %d\n", item_raw, item));
        // Returns null if item is not of type item_type (basically is a group item)
        if (item != NULL) {
            //ACE_DEBUG((LM_DEBUG, "rooms::eventFilter: toolTip updating item\n", help_event->x(), help_event->y()));
            item->update_tip();
            QToolTip::showText(help_event->globalPos(), item->get_tip());
            return true;
        }
    }

    return super::eventFilter(obj, event);
}

} // ns view
} // ns gui
