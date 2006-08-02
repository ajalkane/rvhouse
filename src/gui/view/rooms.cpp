#include <sstream>
#include <functional>

#include <fx.h>

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

FXDEFMAP(room_item) room_item_map[]={
                                        FXMAPFUNC(SEL_QUERY_TIP,0,room_item::on_query_tip),
                                    };
FXDEFMAP(rooms) rooms_map[]={
                                FXMAPFUNC(SEL_QUERY_TIP,0,rooms::on_query_tip),
                                FXMAPFUNC(SEL_DOUBLECLICKED,0,rooms::on_room_doubleclick),
                            };

FXIMPLEMENT(rooms, FXIconList, rooms_map, ARRAYNUMBER(rooms_map))
FXIMPLEMENT(room_item, FXIconItem, room_item_map, ARRAYNUMBER(room_item_map))


room_item::room_item(
    const chat_gaming::room::id_type &rid
) : _room_id(rid), _tip_str("<Empty>")
{
    // Set the data pointer to point to oneself by default, so that finding
    // the item (and therefore it's index) is easy
    setData(this);
}

room_item::room_item() {}

long
room_item::on_query_tip(FXObject *sender,FXSelector sel,void *ptr) {
    _resolve_tip();
    sender->handle(this,FXSEL(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE),
                   (void*)&_tip_str);
    return 1;
}

bool
room_item::room_remove(const chat_gaming::room::id_type &id, int grp) {
    _room_feed.erase_feed(grp);
    ACE_DEBUG((LM_DEBUG, "room_item::room_remove grp %d, num feeds left %d\n",
               grp, _room_feed.feeds_size()));
    return (_room_feed.feeds_size() ? false : true);
}


bool
room_item::resolve_room_state() {
    bool ret = false;

    _resolve_columns() && (ret = true);
    // _resolve_tip()     && (ret = true);

    // At least for now always return true to force an update
    ret = true;

    return ret;
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
    _tip_str.trim();

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
    std::ostringstream str;

    using multi_feed::value;
    using multi_feed::room_value_accessor;
    using multi_feed::user_id_to_string;
    using multi_feed::bool_to_yes_no_string;
    using multi_feed::streamable_to_string;
    using multi_feed::players_slash_max_by_room_id_fobj;
    const multi_feed::room_item &rf = _room_feed; // Just a shorter alias

    str << value(rf, room_value_accessor(&room_type::topic)) << "\t"
    << value(rf, room_owner_id_accessor(user_id_to_string())) << "\t"
    << value(rf, players_slash_max_by_room_id_fobj()) << "\t"
    << value(rf, room_value_accessor(&room_type::laps,
                                     streamable_to_string<size_t>())) << "\t"
    << value(rf, room_value_accessor(&room_type::pickups,
                                     bool_to_yes_no_string()));

    setText(FXString(str.str().c_str()));

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
        setMiniIcon(NULL);
    } else {
        setMiniIcon(app_icons()->get("locked_small"));
    }

    return true;
}

rooms::rooms(
    FXComposite *c, FXObject *tgt,
    FXSelector sel, FXuint opts,
    FXint x, FXint y, FXint w, FXint h)
        : FXIconList(c, tgt, sel, opts, x, y, w, h),
        _observer(NULL)
{
    ACE_DEBUG((LM_DEBUG, "rooms opts: %d\n", opts));

    appendHeader(langstr("words/topic"),   NULL, 200);
    appendHeader(langstr("words/host"),    NULL, 100);
    appendHeader(langstr("words/players"), NULL, 50);
    appendHeader(langstr("words/laps"),    NULL, 35);
    appendHeader(langstr("words/pickups"), NULL, 50);
}

void
rooms::create() {
    super::create();
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
    if (item->resolve_room_state())
        this->updateItem(item);

    // Check if the room should be disabled (if the host
    // is playing then so should be done)
    std::string status = multi_feed::value(
                             item->feed_item(),
                             multi_feed::room_owner_id_accessor(multi_feed::user_id_to_status_string())
                         );

    int disable_status = chat_gaming::user::status_playing;
    if (status == chat_gaming::user::status_to_string(disable_status)) {
        disableItem(item);
    } else {
        enableItem(item);
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
            if (item->resolve_room_state())
                this->updateItem(item);
        }
    }
}

void
rooms::_remove_dropped() {
    ACE_DEBUG((LM_DEBUG, "rooms::_remove_dropped\n"));
    room_item *item = NULL;
    while ((item = _room_item_map.pop_dropped())) {
        ACE_DEBUG((LM_DEBUG, "rooms::_remove_dropped: item %d\n", item));
        // removeItem deletes the allocated item also.
        removeItem(item);
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

    super::appendItem(item);

    ACE_DEBUG((LM_DEBUG, "rooms::_new_user: resolve\n"));
    _update_room_state(item);
    // item->resolve_room_state();

    if (_observer) _observer->room_added(r);
}

int
rooms::selected_item_index() const {
    int i = 0;
    int c = this->getNumItems();
    const item_type *item;
    for (; i < c; i++) {
        item = item_at(i);
        if (item->isSelected()) return i;
    }
    return -1;
}

long
rooms::on_query_tip(FXObject *sender,FXSelector sel,void *ptr) {
    // ACE_DEBUG((LM_DEBUG, "rooms::on_query_tip called!\n"));

    FXint x,y,i; FXuint buttons;
    getCursorPosition(x,y,buttons);
    FXIconItem *item = ((i = getItemAt(x,y)) >= 0 ? getItem(i) : NULL);
    if (item) {
        if (item->handle(sender, sel, ptr)) return 1;
        // else ACE_DEBUG((LM_DEBUG, "rooms::on_query_tip: target did not handle\n"));
    }

    return 0;
}

long
rooms::on_room_doubleclick(FXObject *sender,FXSelector sel,void *ptr) {
    ACE_DEBUG((LM_DEBUG, "rooms::on_room_doubleclick called!\n"));

    if (_handler_item_doubleclicked.target) {
        return _handler_item_doubleclicked.target->handle(
                   this,
                   _handler_item_doubleclicked.sel,
                   ptr
               );
    }

    return 0;
}

} // ns view
} // ns gui
