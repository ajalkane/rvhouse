#include "message_room_join.h"

message_room_join::message_room_join(
    int t, 
    const chat_gaming::room::id_type &rid, 
    const std::string &pass,
    const chat_gaming::user::id_type &uid,
    unsigned seq,
    int gb
) : message_grouped(t, uid, seq, gb), _room_id(rid), _password(pass)
{
}

message *
message_room_join::duplicate() {
    return new message_room_join(id(), room_id(), password(), sender_id(), sequence(), group_base());
}
