#ifndef _MULTI_FEED_ROOM_MAP_H_
#define _MULTI_FEED_ROOM_MAP_H_

#include <string>
#include <map>
#include <list>
#include <functional>

#include "../chat_gaming/room.h"
#include "../model/house.h"
#include "../main.h"

#include "common.h"
#include "algorithm.h"

/**
 * Multi Feed namespace provides functions that ease merging 
 * information from several feeds together... in the context
 * of rv_house, there is centralized server and decentralized peer to peer
 * network that are used to gather usually overlapping information
 */
namespace multi_feed {
    // Maps room id to a unique pointer of type T. Pointers used instead
    // of iterators to simplify the implementation.
    // room_map is basically a wrapper around std::map with semantic
    // differences... it is only used to provide similar interface as
    // user_map.
    template <class T>
    class room_map {
    public:
        typedef chat_gaming::room::id_type id_type;
        typedef T* value_type;
        typedef std::map<id_type, value_type> id_map_type;
        typedef std::list<value_type>         drop_list_type;
        
        value_type find(const id_type &rid) const {
            ACE_DEBUG((LM_DEBUG, "multi_feed::room_map::find %s\n",
                      rid.c_str()));
            
            typename id_map_type::const_iterator ri 
              = _id_map.find(rid);
              
            if (ri != _id_map.end()) {
                ACE_DEBUG((LM_DEBUG, "multi_feed::room_map::find found item\n"));
                return ri->second;
            }
            return NULL;
        }
        
        // Slow operation
        id_type find_key(const value_type &value) const {
            typename id_map_type::const_iterator i = _id_map.begin();
            for (; i != _id_map.end(); i++) {
                if (i->second == value) return i->first;
            }
            return id_type();
        }
        
        size_t erase(const id_type &rid) {
            size_t n = 0;
            ACE_DEBUG((LM_DEBUG, "multi_feed::room_map::erase %s\n",
                      rid.c_str()));

            typename id_map_type::iterator ri 
              = _id_map.find(rid);
              
            if (ri != _id_map.end()) {
                ACE_DEBUG((LM_DEBUG, "multi_feed::room_map::erase found "
                           "from id_map\n"));
                _drop_list.push_back(ri->second);
                _id_map.erase(ri);
                n++;
            }               
            
            return n;
        }
        
        bool insert(const id_type &rid, value_type val) {
            ACE_DEBUG((LM_DEBUG, "multi_feed::user_map::insert '%s'\n",
                      rid.c_str()));
            if (!rid.empty()) {         
                std::pair<typename id_map_type::iterator, bool> ires = 
                  _id_map.insert(std::make_pair(rid, val));
                                                
                return ires.second;
            } else {
                ACE_ERROR((LM_ERROR, "multi_feed::user_map::insert empty room id???\n"));
            }
        
            return false;
        }
        
        // Resolves the mappings so that there won't be duplicates
        // A no-op in room_map as duplicates are not possible.
        inline void resolve(const id_type &rid) {
        }   
        
        value_type pop_dropped() {
            typename drop_list_type::iterator i = _drop_list.begin();
            if (i != _drop_list.end()) {
                value_type vt = *i;
                _drop_list.pop_front();
                return vt;
            }
            return NULL;
        }
        
    private:
        id_map_type    _id_map;
        drop_list_type _drop_list;      
    };
} // ns multi_feed

#endif /*_MULTI_FEED_ROOM_MAP_H_*/
