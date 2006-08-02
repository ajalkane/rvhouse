#include "message_send_room.h"

message_send_room::message_send_room(
    int t,
    const std::string &str,
    const chat_gaming::user::id_type &uid,
    const chat_gaming::room::id_type &rid,
    unsigned s,
    int gb
) : message_send(t, str, uid, s, gb), _room_id(rid)
{
}

message *
message_send_room::duplicate() {
    return new message_send_room(id(), str(), sender_id(), room_id(), sequence(), group_base());
}
