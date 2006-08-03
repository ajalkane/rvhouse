#ifndef _MULTI_FEED_ROOM_UTIL_H_
#define _MULTI_FEED_ROOM_UTIL_H_

#include <map>
#include <string>
#include <sstream>
#include <functional>
#include <iterator>

#include <netcomgrp/common.h>

#include "../common.h"
#include "../chat_gaming/room.h"
#include "../model/house.h"
#include "../main.h"

#include "common.h"

namespace multi_feed {

class players_slash_max_by_room_id_fobj {
    std::string _cache;
public: 
    std::string operator()(const chat_gaming::room::id_type &rid, int grp) {
        if (!_cache.empty()) return _cache;
        
        model::house::house_type::room_iterator ri 
          = house_model()->room_find(rid, grp);
        if (ri == house_model()->room_end()) return std::string();
        
        size_t pl_max = ri->max_players();

        // Group is of no importance in amount of players in the room, 
        // collect the maximum
        size_t pl     = house_model()->room_size(rid);
        
        std::ostringstream oss;
        oss << pl << "/" << pl_max;
        _cache = oss.str();
        return _cache;
    }   
};

// Returns stringified ids of users in the room. The iterator must be able
// to handle strings
template <typename IteratorType>
size_t users_in_room(const chat_gaming::room::id_type &id, IteratorType &out) {
    typedef std::map<std::string, bool>           users_map_type;
    typedef std::list<chat_gaming::user::id_type> user_ids_list_type;
    
    users_map_type users_collected;

    const 
    model::house::group_desc_type &g = house_model()->group_desc();
    model::house::group_desc_type::const_iterator gi = g.begin();
    for (; gi != g.end(); gi++) {
        user_ids_list_type user_ids;
        std::insert_iterator<user_ids_list_type> 
          ins_itr(user_ids, user_ids.begin());
          
        house_model()->user_ids_in_room(id, ins_itr, gi->first);
        
        // From user ids that have id_str (login_id basically) set, insert 
        // into the list those that are not there yet
        user_ids_list_type::iterator ui = user_ids.begin();
        for (; ui != user_ids.end(); ui++) {
            if (ui->id_str().empty() == false &&
                users_collected.count(ui->id_str()) == 0)
            {
                users_collected[ui->id_str()] = true;
                *out++ = ui->id_str();
            }
        }
    }
    
    return users_collected.size();  
}

} // multi_feed

#endif /*_MULTI_FEED_ROOM_UTIL_H_*/
