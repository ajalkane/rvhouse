#ifndef _MESSAGING_MESSAGE_ROOM_H_
#define _MESSAGING_MESSAGE_ROOM_H_

#include "../chat_gaming/room.h"

#include "message.h"
#include "message_grouped.h"

class message_room : public message_grouped {
    chat_gaming::room _room;
public:
    message_room(int msg_type, const chat_gaming::room &r,
                 const chat_gaming::user::id_type &uid,
                 unsigned seq, int group_base);
    virtual message *duplicate();

    inline const chat_gaming::room &room() const;
};

inline const chat_gaming::room &
message_room::room() const { return _room; }

#endif //_MESSAGING_MESSAGE_ROOM_H_
