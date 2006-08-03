#ifndef _MULTI_FEED_UTIL_H_
#define _MULTI_FEED_UTIL_H_

#include "common.h"
#include "algorithm.h"
#include "config.h"

namespace multi_feed {

std::string header_value(
    const std::string &hdr,
    const feed_value_map_type &vals
);

std::string header_value(
    const std::string &hdr,
    const std::string &value
);

// Constructs the header/value pair string using an accessor to object
template <class FeedItem, class Accessor>
std::string header_value(
    const FeedItem &feed_item,
    const std::string &hdr,
    Accessor acc
) {
    multi_feed::feed_value_map_type vals;
    std::string val = feed_item.values(vals, acc);

    if (config::always_display_feeds) return header_value(hdr, vals);
        
    return (val.empty() == false || vals.size() == 0 
            ? header_value(hdr, val)
            : header_value(hdr, vals));
}   

template <class FeedItem, class Accessor>
std::string header_non_empty_value(
    const FeedItem &feed_item,
    const std::string &hdr,
    Accessor acc
) {
    multi_feed::feed_value_map_type vals;
    std::string val = feed_item.values(vals, acc);

    if (!val.empty()) {
        if (config::always_display_feeds) return header_value(hdr, vals);   
        return header_value(hdr, val); // "user hasn't validated!");
    }
    else if (vals.size() > 0) {
        return header_value(hdr, vals);
    }
    return std::string();
}   

template <class FeedItem, class Accessor>
inline
std::string value(
    const FeedItem &feed_item,
    Accessor acc
) {
    return header_value(feed_item, std::string(), acc);
}   

// Returns a unique value
template <class FeedItem, class Accessor>
std::string uniq_value(
    const FeedItem &feed_item,
    Accessor acc
) {
    multi_feed::feed_value_map_type vals;
    std::string val = feed_item.values(vals, acc);
        
    if (val.empty() == false || vals.size() == 0)
        return val;
    else
        return vals.begin()->second;
}   

} // ns multi_feed

#endif /*_MULTI_FEED_UTIL_H_*/
