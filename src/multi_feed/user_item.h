#ifndef _MULTI_FEED_USER_ITEM_H_
#define _MULTI_FEED_USER_ITEM_H_

#include <string>

#include "../chat_gaming/user.h"
#include "item.h"

/**
 * Multi Feed namespace provides functions that ease merging 
 * information from several feeds together... in the context
 * of rv_house, there is centralized server and decentralized peer to peer
 * network that are used to gather usually overlapping information
 */
namespace multi_feed {
class user_item : public item<chat_gaming::user::id_type> {
public:
    typedef item<chat_gaming::user::id_type> super;
    typedef chat_gaming::user user_type;

    std::string unambiguous_display_id() const;
};

} // ns multi_feed

#endif /*_MULTI_FEED_USER_ITEM_H_*/
