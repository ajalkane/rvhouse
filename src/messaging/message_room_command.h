#ifndef _MESSAGING_MESSAGE_ROOM_COMMAND_H_
#define _MESSAGING_MESSAGE_ROOM_COMMAND_H_

#include <string>

#include "../chat_gaming/room.h"
#include "../chat_gaming/user.h"

#include "message.h"
#include "message_grouped.h"

class message_room_command : public message_grouped {
    chat_gaming::room::id_type _room_id;
    chat_gaming::user::id_type _target_user_id;

public:
    message_room_command(int msg_type,
                         const chat_gaming::room::id_type &rid,
                         const chat_gaming::user::id_type &tid,
                         const chat_gaming::user::id_type &sid,
                         unsigned s,
                         int group_base);

    virtual message *duplicate();

    inline const chat_gaming::user::id_type &target_user_id() const {
        return _target_user_id;
    }

    inline const chat_gaming::room::id_type &room_id() const {
        return _room_id;
    }
};

#endif //_MESSAGING_MESSAGE_ROOM_COMMAND_H_
