#ifndef _MESSAGING_MESSAGE_SEND_ROOM_H_
#define _MESSAGING_MESSAGE_SEND_ROOM_H_

#include <string>

#include "../chat_gaming/room.h"

#include "message_send.h"

class message_send_room : public message_send {
    chat_gaming::room::id_type _room_id;
public: 
    message_send_room(int msg_type, 
                     const std::string &str,
                     const chat_gaming::user::id_type &uid, 
                     const chat_gaming::room::id_type &rid,
                     unsigned seq,                   
                     int group_base);
                 
    virtual message *duplicate();
    
    inline const chat_gaming::room::id_type &room_id() const { return _room_id; }
};

#endif //_MESSAGING_MESSAGE_SEND_ROOM_H_
