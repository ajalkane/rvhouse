#ifndef _MULTI_FEED_ROOM_ITEM_H_
#define _MULTI_FEED_ROOM_ITEM_H_

#include "../chat_gaming/room.h"
#include "item.h"

/**
 * Multi Feed namespace provides functions that ease merging 
 * information from several feeds together... in the context
 * of rv_house, there is centralized server and decentralized peer to peer
 * network that are used to gather usually overlapping information
 */
namespace multi_feed {
    class room_item : public item<chat_gaming::room::id_type> {
    public:
        typedef item<chat_gaming::room::id_type> super;
        typedef chat_gaming::room room_type;        
    };

} // ns multi_feed

#endif /*_MULTI_FEED_ROOM_ITEM_H_*/
