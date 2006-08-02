#ifndef _MODEL_HOUSE_H_
#define _MODEL_HOUSE_H_

#include <map>

#include "../common.h"
#include "../chat_gaming/house.h"

namespace model {

/**
 * Keeps a model of the chat house for the purposes of GUI...
 * Due to the multi-source (DHT, centralized) nature of the 
 * possible data, a couple of copies are kept and not a full
 * interface to chat_gaming::house etc is not given.
 */
class house {
public:
    typedef chat_gaming::room  room_type;
    typedef chat_gaming::user  user_type;
    typedef chat_gaming::house house_type;
    typedef chat_gaming::house::user_iterator user_iterator;
    typedef chat_gaming::house::room_iterator room_iterator;

    typedef std::map<int, std::string> group_desc_type;

    house();

    inline void allow_group(const std::string &dsc, int grp) {
        _grp_dsc_map[grp] = dsc;
        _grp_str_map[grp] = _str_id_map_type();
        _house_map[grp]   = house_type();
    }

    inline const group_desc_type &group_desc() {
        return _grp_dsc_map;
    }

    // Updates and adds if does not exist
    house_type::user_iterator user_update(const user_type &u, int grp);
    // Erases and/or does a nasty dance.
    bool user_erase(const user_type::id_type &id, int grp);
    // If grp not specifies tries finding from all groups until some
    // found (order arbitrary)
    // inline user_iterator user_find(const UserType &u, int grp = -1);
    house_type::user_iterator user_find(const user_type::id_type &id);
    house_type::user_iterator user_find(const user_type::id_type &id, int grp);
    // inline user_iterator user_begin();
    // This should be used only for comparing if user_find did not find
    // anything matching
    inline house_type::user_iterator user_end() {
        return _house_map.begin()->second.user_end();
    }

    template <typename IteratorType>
    int user_ids_in_room(const room_type::id_type &id, IteratorType &out, int grp) {
        house_type &h = _house_find(grp);
        house_type::user_iterator i = h.user_begin();
        int count = 0;
        for (; i != h.user_end(); i++) {
            if (i->room_id() == id) {
                *out++ = i->id();
                count++;
            }
        }
        return count;
    }

    // Updates and adds if does not exist
    house_type::room_iterator room_update(const room_type &u, int grp);
    // Erases and/or does a nasty dance.
    bool room_erase(const room_type::id_type &id, int grp);
    // If grp not specifies tries finding from all groups until some
    // found (order arbitrary)
    // inline room_iterator room_find(const UserType &u, int grp = -1);
    house_type::room_iterator room_find(const room_type::id_type &id);
    house_type::room_iterator room_find(const room_type::id_type &id, int grp);

    // Returns number of players in the room
    size_t room_size(const room_type::id_type &id);

    // inline room_iterator room_begin();
    // This should be used only for comparing if room_find did not find
    // anything matching
    inline house_type::room_iterator room_end() {
        return _house_map.begin()->second.room_end();
    }

    // TODO find/erase by id rather than by type
    // inline room_iterator room_insert(const RoomType &u);
    // inline bool room_erase(const RoomType &u);
    // inline bool room_erase(room_iterator ui);
    // inline room_iterator room_find(const RoomType &u);
    // inline room_iterator room_find(const typename RoomType::id_type &u);
    // inline room_iterator room_begin();
    // inline room_iterator room_end();

    inline house_type &house_find(int grp) {
        return _house_find(grp);
    }

protected:
    typedef std::map<int, house_type> _house_map_type;
    typedef std::map<std::string, user_type::id_type> _str_id_map_type;
    typedef std::map<int, _str_id_map_type>           _grp_str_map_type;

    _house_map_type   _house_map;
    _grp_str_map_type _grp_str_map;
    group_desc_type   _grp_dsc_map;

    house_type::user_iterator _user_find(
        const user_type::id_type &id, int grp, house_type &h
    );

    house_type &_house_find(int grp);
};

} // ns model

#endif //_MODEL_HOUSE_H_
