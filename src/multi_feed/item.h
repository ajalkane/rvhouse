#ifndef _MULTI_FEED_ITEM_H_
#define _MULTI_FEED_ITEM_H_

#include <string>
#include <map>
#include <functional>

#include "common.h"
#include "algorithm.h"

/**
 * Multi Feed namespace provides functions that ease merging 
 * information from several feeds together... in the context
 * of rv_house, there is centralized server and decentralized peer to peer
 * network that are used to gather usually overlapping information
 */
namespace multi_feed {
template <class ItemId>
class item {
public:
    typedef ItemId item_id_type;
    typedef std::map<int, item_id_type> feed_id_map_type;

    template <class Accessor>
    std::string values(
        feed_value_map_type &vals,
        Accessor acc
    ) const {
        std::string val;
        bool all_equal_values = true;

        typename feed_id_map_type::const_iterator i = _feed_id_map.begin();
        for (; i != _feed_id_map.end(); i++) {
            const std::string &s = acc(i->second, i->first);
            // Removing empty/not set values
            if (!s.empty()) {
                vals[i->first] = s;
                // Checking if all the values are the same
                if (val.empty()) val = s;
                else if (val != s) all_equal_values = false;
            }
        }

        ACE_DEBUG((LM_DEBUG, "multi_feed::item::values: all_equal_values: %d\n", all_equal_values));
        return (all_equal_values ? val : std::string());
    }

    inline void set_feed(const item_id_type &id, int feed) {
        _feed_id_map[feed] = id;
    }
    inline typename feed_id_map_type::const_iterator find(int feed) {
        return _feed_id_map.find(feed);
    }

    inline typename feed_id_map_type::const_iterator begin() {
        return _feed_id_map.begin();
    }
    inline typename feed_id_map_type::const_iterator end() {
        return _feed_id_map.end();
    }

    inline void erase_feed(int feed) {
        size_t ret = _feed_id_map.erase(feed);
        // TODO away debugging:
        ACE_DEBUG((LM_DEBUG, "multi_feed::item::erase: %d feed %d returned %d\n",
                   this, feed, ret));
        typename feed_id_map_type::iterator i = _feed_id_map.begin();
        for (; i != _feed_id_map.end(); i++) {
            ACE_DEBUG((LM_DEBUG, "multi_feed::item::erase: grp/value: "
                       "%d/%s\n", i->first, i->second.c_str()));
        }

    }
    inline size_t feeds_size() const {
        return _feed_id_map.size();
    }

private:
    // Mapping from feed ids to item ids
    feed_id_map_type _feed_id_map;
};
} // ns multi_feed

#endif /*ITEM_H_*/
