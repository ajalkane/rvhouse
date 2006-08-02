#include "message_room_command.h"

message_room_command::message_room_command(
    int t,
    const chat_gaming::room::id_type &rid,
    const chat_gaming::user::id_type &tid,
    const chat_gaming::user::id_type &sid,
    unsigned seq,
    int gb
) : message_grouped(t, sid, seq, gb), _room_id(rid), _target_user_id(tid)
{
}

message *
message_room_command::duplicate() {
    return new message_room_command(
               id(), room_id(), target_user_id(),
               sender_id(), sequence(), group_base()
           );
}
