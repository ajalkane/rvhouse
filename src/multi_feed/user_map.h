#ifndef _MULTI_FEED_USER_MAP_H_
#define _MULTI_FEED_USER_MAP_H_

#include <string>
#include <map>
#include <list>
#include <functional>

#include "../chat_gaming/user.h"
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
    // Maps user id to a unique pointer of type T. Pointers used instead
    // of iterators to simplify the implementation.
    template <class T>
    class user_map {
    public:
        typedef chat_gaming::user::id_type id_type;
        typedef std::string                id_key_type;
        typedef id_type                    unid_key_type;
        typedef T* value_type;
        typedef std::pair<id_type, value_type>    pair_type;
        typedef std::map<id_key_type, value_type> id_map_type;
        typedef std::list<pair_type>              unid_list_type;
        typedef std::list<value_type>             drop_list_type;
        
        value_type find(const id_type &uid) {
            ACE_DEBUG((LM_DEBUG, "multi_feed::user_map::find %s\n",
                      uid.c_str()));
            value_type vt = NULL;
            
            if (!uid.id_str().empty()) {
                typename id_map_type::iterator ui 
                  = _id_map.find(uid.id_str());
                  
                if (ui != _id_map.end()) {
                    vt = ui->second;                    
                }
            } else {
                ACE_DEBUG((LM_DEBUG, "multi_feed::user_map::no name set, "
                          "finding from unassigned id list\n"));
                typename unid_list_type::iterator i = std::find_if(
                    _unid_list.begin(),
                    _unid_list.end(),
                    _pair_1st_equals(uid)
                );
            
                if (i != _unid_list.end()) {
                    vt = i->second;
                }
            }

            if (vt) {
                ACE_DEBUG((LM_DEBUG, "multi_feed::user_map::find found item\n"));
            }
            
            return vt;          
        }
        
        size_t erase(const id_type &uid) {
            size_t n = 0;
            ACE_DEBUG((LM_DEBUG, "multi_feed::user_map::erase %s\n",
                      uid.c_str()));
            if (!uid.id_str().empty()) {
                typename id_map_type::iterator ui 
                  = _id_map.find(uid.id_str());
                  
                if (ui != _id_map.end()) {
                    ACE_DEBUG((LM_DEBUG, "multi_feed::user_map::erase found "
                               "from uid_map\n"));
                    _drop_list.push_back(ui->second);
                    _id_map.erase(ui);
                    n++;
                }               
            } 

            typename unid_list_type::iterator i = std::find_if(
                _unid_list.begin(),
                _unid_list.end(),
                _pair_1st_equals(uid)
            );
        
            if (i != _unid_list.end()) {
                ACE_DEBUG((LM_DEBUG, "users_view::erase: "
                          "id found from unid_list, "
                          "dropping\n"));
                _drop_list.push_back(i->second);
                _unid_list.erase(i);
                n++;
            }
            
            return n;
        }
        
        bool insert(const id_type &uid, value_type val) {
            ACE_DEBUG((LM_DEBUG, "multi_feed::user_map::insert '%s'\n",
                      uid.c_str()));
            if (!uid.id_str().empty()) {            
                std::pair<typename id_map_type::iterator, bool> ires = 
                  _id_map.insert(std::make_pair(uid.id_str(), val));
                                                
                return ires.second;
            } else {
                ACE_DEBUG((LM_DEBUG, "multi_feed::user_map::insert pushing to unid_list\n",
                          uid.c_str()));
                _unid_list.push_back(pair_type(uid, val));
                return true;
            }
        
            return false;
        }
        
        // Resolves the user mappings so that there won't be duplicates
        void resolve(const id_type &uid) {
            ACE_DEBUG((LM_DEBUG, "multi_feed::user_map::resolve '%s'\n",
                      uid.c_str()));
            if (!uid.id_str().empty() && 
                _id_map.find(uid.id_str()) != _id_map.end()) 
            {
                // If name set and found from id map, can remove it from 
                // unassiged map
            
                typename unid_list_type::iterator i = std::find_if(
                    _unid_list.begin(),
                    _unid_list.end(),
                    _pair_1st_equals(uid)
                );
        
                if (i != _unid_list.end()) {
                    ACE_DEBUG((LM_DEBUG, "users_view::resolve: "
                              "id found from unid_list, "
                              "dropping\n"));
                    _drop_list.push_back(i->second);
                    _unid_list.erase(i);
                }
            }
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
        unid_list_type _unid_list;
        drop_list_type _drop_list;
        
        class _pair_1st_equals
          : public std::unary_function<bool, pair_type> {
            const id_type &_comp;
        public:
            inline _pair_1st_equals(const id_type &comp)
                : _comp(comp) {}
            inline bool operator()(const pair_type &p) {
                return _comp == p.first;
            }
        };              
    };
} // ns multi_feed

#endif /*_MULTI_FEED_USER_MAP_H_*/
