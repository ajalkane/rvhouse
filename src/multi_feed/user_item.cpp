#include "user_item.h"
#include "user_accessors.h"

namespace multi_feed {
    
std::string
user_item::unambiguous_display_id() const {
    feed_value_map_type user_ids;
    std::string user_id = values(user_ids, user_value_accessor(&user_type::login_id));
    if (user_id.empty()) {
        user_id = values(user_ids, user_value_accessor(&user_type::display_id));
    }
    if (user_id.empty())
        user_id = "?";
    
    return user_id;
}

} // ns multi_feed
