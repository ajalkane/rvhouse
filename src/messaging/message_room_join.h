#ifndef _MESSAGING_MESSAGE_ROOM_JOIN_H_
#define _MESSAGING_MESSAGE_ROOM_JOIN_H_

#include <string>

#include "../chat_gaming/room.h"
#include "../chat_gaming/user.h"

#include "message.h"
#include "message_grouped.h"

class message_room_join : public message_grouped {
    chat_gaming::room::id_type _room_id;
    std::string                _password;
public:
    message_room_join(int msg_type,
                      const chat_gaming::room::id_type &rid,
                      const std::string &pass,
                      const chat_gaming::user::id_type &uid,
                      unsigned seq,
                      int group_base);

    virtual message *duplicate();

    inline const chat_gaming::room::id_type &room_id() const {
        return _room_id;
    }
    inline const std::string &password() const {
        return _password;
    }
};

#endif //_MESSAGING_MESSAGE_ROOM_JOIN_H_
