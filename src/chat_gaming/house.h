#ifndef _CHAT_GAMING_HOUSE_H_
#define _CHAT_GAMING_HOUSE_H_

#include <string>
#include <chat/house.h>
#include "user.h"
#include "room.h"

namespace chat_gaming {
    class house : public chat::house<user, room> {
        typedef chat::house<user, room> super;
    public:
        // Tries finding a matching user by its id string representation
        // There's no caching or anything because one-on-one communication
        // should happen relatively infrequently. If this should change, 
        // this can be changed quite easily.
        user_iterator user_find(const std::string &id_str) {
            user_iterator i = user_begin();
            for (; i != user_end(); i++) {
                if (i->id().id_str() == id_str) break;
            }
            return i;
        }
        // ^ the above version masks base classes user_find versions
        // so reintroduce them:
        inline user_iterator user_find(const user_type &u) {
            return super::user_find(u);
        }
        inline user_iterator user_find(const user_type::id_type &u) {
            return super::user_find(u);
        }
    };
} // ns chat

#endif //_CHAT_HOUSE_H_
