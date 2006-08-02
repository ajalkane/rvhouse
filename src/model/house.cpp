#include <string>

#include "../exception.h"
#include "house.h"

namespace model {

house::house() {

}

// Updates and adds if does not exist
house::house_type::user_iterator
house::user_update(const house::user_type &u, int grp) {
    ACE_DEBUG((LM_DEBUG, "model::house::user_update id '%s'\n",
               u.id().c_str()));
    assert(u.id().node() != NULL);

    house_type &h = _house_find(grp);
    house_type::user_iterator ui = h.user_find(u.id());
    if (ui == h.user_end()) {
        ACE_DEBUG((LM_DEBUG, "model::house user (id %s) not found from "
                   "group %d, adding\n", u.id().c_str(), grp));
        ui = h.user_insert(u);
        if (!u.id().id_str().empty()) {
            ACE_DEBUG((LM_DEBUG, "model::house creating mapping %s -> %d\n",
                       u.id().id_str().c_str(), u.id().node()));

            // If id specified, make a mapping string -> id to faciliate
            // finding users just by their string id.
            _grp_str_map[grp].insert(
                std::make_pair(u.id().id_str(), u.id())
            );
        }
    } else {
        ACE_DEBUG((LM_DEBUG, "model::house user (id %s) found from "
                   "group %d, updating\n", u.id().c_str(), grp));
        // Do not update if the received user has no id_str() set
        if (u.id().id_str().empty()) {
            ACE_DEBUG((LM_DEBUG, "model::house received update "
                       "did not have user id string set, no update\n"));
            return ui;
        }
        // If id strings differ, then have to erase and insert the
        // entry to the map, because the order of the elements is potentially
        // changed because id changes. This should happen only when the old
        // id_str was empty (not known yet) and it became known after
        // an update.
        if (ui->id().id_str() != u.id().id_str()) {
            ACE_DEBUG((LM_DEBUG, "model::house old user (id %s) differs, "
                       "erasing and inserting\n", ui->id().c_str()));
            // Have to erase/insert the string -> node mapping also
            _grp_str_map[grp].erase(ui->id());
            _grp_str_map[grp].insert(
                std::make_pair(u.id().id_str(), u.id())
            );
            h.user_erase(ui);
            h.user_insert(u);
        } else {
            *ui = u;
        }
    }

    return ui;
}

// Erases and/or does a nasty dance.
bool
house::user_erase(const house::user_type::id_type &id, int grp) {
    ACE_DEBUG((LM_DEBUG, "model::house::user_erase id %s\n", id.c_str()));
    house_type &h = _house_find(grp);
    bool r = h.user_erase(id);
    // Will erase from the mapping also.
    _grp_str_map[grp].erase(id);
    if (!r) ACE_DEBUG((LM_WARNING, "model::house WARNING could not erase "
                           "user id %s from group %d\n", id.c_str(), grp));
    return r;
}


house::house_type::user_iterator
house::user_find(const user_type::id_type &id) {
    _house_map_type::iterator hi = _house_map.begin();
    for (; hi != _house_map.end(); hi++) {
        const int &grp = hi->first;
        house_type &h = hi->second;
        house_type::user_iterator ui = _user_find(id, grp, h);
        if (ui != h.user_end()) return ui;
    }

    ACE_DEBUG((LM_DEBUG, "model::house user_find no match found for id %s\n",
               id.c_str()));

    // Always use the first one as the end marker.
    return _house_map.begin()->second.user_end();
}

house::house_type::user_iterator
house::user_find(const user_type::id_type &id, int grp) {
    ACE_DEBUG((LM_DEBUG, "model::house::user_find id '%s' from group %d\n",
               id.c_str(), grp));

    house_type &h = _house_find(grp);
    house_type::user_iterator ui = _user_find(id, grp, h);
    // TODO away
    if (ui == h.user_end()) {
        ACE_DEBUG((LM_DEBUG, "model::house::user_find no match found\n"));
    }
    return (ui == h.user_end() ? user_end() : ui);
}

house::house_type::user_iterator
house::_user_find(const user_type::id_type &id, int grp, house_type &h) {
    // Search id
    user_type::id_type sid = id;
    ACE_DEBUG((LM_DEBUG, "model::house user_find %s from group %d\n",
               sid.c_str(), grp));
    // Often the searches are done by only the id string, in which case
    // the cached node is needed to be supplied
    if (sid.node() == NULL) {
        ACE_DEBUG((LM_DEBUG, "model::house user_find finding by str id %s\n",
                   sid.id_str().c_str()));
        _str_id_map_type &sidmap = _grp_str_map[grp];
        _str_id_map_type::iterator si = sidmap.find(sid.id_str());
        if (si != sidmap.end()) {
            ACE_DEBUG((LM_DEBUG, "model::house user_find setting search "
                       "node to %d\n", si->second.node()));
            sid.node(si->second.node());
        } else {
            ACE_DEBUG((LM_DEBUG, "model::house user_find no node found\n"));
        }
    }

    return h.user_find(sid);
}

/**********************************
 * ROOM
 **********************************/

// Updates and adds if does not exist
house::house_type::room_iterator
house::room_update(const house::room_type &u, int grp) {
    ACE_DEBUG((LM_DEBUG, "model::house::room_update id '%s'\n",
               u.id().c_str()));

    house_type &h = _house_find(grp);
    house_type::room_iterator ui = h.room_find(u.id());
    if (ui == h.room_end()) {
        ACE_DEBUG((LM_DEBUG, "model::house room (id %s) not found from "
                   "group %d, adding\n", u.id().c_str(), grp));
        ui = h.room_insert(u);
    } else {
        ACE_DEBUG((LM_DEBUG, "model::house room (id %s) found from "
                   "group %d, updating\n", u.id().c_str(), grp));
        *ui = u;
    }

    if (ui == h.room_end()) {
        ACE_ERROR((LM_ERROR, "model::house room (id %s) failed to update in "
                   "group %d\n", u.id().c_str(), grp));
        ui = room_end();
    }

    return ui;
}

// Erases and/or does a nasty dance.
bool
house::room_erase(const house::room_type::id_type &id, int grp) {
    house_type &h = _house_find(grp);
    bool r = h.room_erase(id);
    if (!r) ACE_DEBUG((LM_WARNING, "model::house WARNING could not erase "
                           "room id %s from group %d\n", id.c_str(), grp));
    return r;
}


house::house_type::room_iterator
house::room_find(const room_type::id_type &sid) {
    ACE_DEBUG((LM_DEBUG, "model::house::room_find id '%s' from all groups\n",
               sid.c_str()));
    _house_map_type::iterator hi = _house_map.begin();
    for (; hi != _house_map.end(); hi++) {
        // Alias to the group for readability
        const int &grp = hi->first;
        ACE_DEBUG((LM_DEBUG, "model::house room_find %s from group %d\n",
                   sid.c_str(), grp));
        house_type &h = hi->second;
        house_type::room_iterator ui = h.room_find(sid);
        if (ui != h.room_end()) return ui;
    }

    ACE_DEBUG((LM_DEBUG, "model::house room_find no match found for id %s\n",
               sid.c_str()));

    // Always use the first one as the end marker.
    return room_end();
}

house::house_type::room_iterator
house::room_find(const room_type::id_type &id, int grp) {
    ACE_DEBUG((LM_DEBUG, "model::house::room_find id '%s' from group %d\n",
               id.c_str(), grp));

    house_type &h = _house_find(grp);
    house_type::room_iterator ri = h.room_find(id);

    return (ri == h.room_end() ? room_end() : ri);
}

size_t
house::room_size(const room_type::id_type &id) {
    ACE_DEBUG((LM_DEBUG, "model::house::room_size id '%s'\n", id.c_str()));
    // Collect all user ids from the groups that match the room id to
    // a hash to determine the size
    std::map<std::string, bool> m;
    _house_map_type::iterator hi = _house_map.begin();
    for (; hi != _house_map.end(); hi++) {
        house_type &h = hi->second;
        house_type::user_iterator ui = h.user_begin();
        for (; ui != h.user_end(); ui++) {
            if (ui->room_id() == id) {
                ACE_DEBUG((LM_DEBUG, "model::house::room_size user '%s' in room\n",
                           ui->id().c_str()));
                if (ui->id().id_str().empty() == false) {
                    m[ui->id().id_str()] = true;
                }
            }
        }
    }
    ACE_DEBUG((LM_DEBUG, "model::house::room_size is %d\n", m.size()));
    return m.size();
}

house::house_type &
house::_house_find(int grp) {
    _house_map_type::iterator hi = _house_map.find(grp);
    if (hi == _house_map.end()) {
        throw exceptionf(0, "model::house: Internal error, group %d "
                         "not allowed", grp);
    }
    return hi->second;
}

// If grp not specifies tries finding from all groups until some
// found (order arbitrary)
// inline user_iterator user_find(const UserType &u, int grp = -1);
// inline room_iterator user_find(const typename UserType::id_type &u);
// inline user_iterator user_begin();
// inline user_iterator user_end();

// TODO find/erase by id rather than by type
// inline room_iterator room_insert(const RoomType &u);
// inline bool room_erase(const RoomType &u);
// inline bool room_erase(room_iterator ui);
// inline room_iterator room_find(const RoomType &u);
// inline room_iterator room_find(const typename RoomType::id_type &u);
// inline room_iterator room_begin();
// inline room_iterator room_end();

} // ns model

