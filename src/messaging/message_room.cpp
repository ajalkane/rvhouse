#include "message_room.h"

message_room::message_room(
    int t,
    const chat_gaming::room &r,
    const chat_gaming::user::id_type &uid,
    unsigned seq, int gb)
        : message_grouped(t, uid, seq, gb), _room(r)
{
    // Set room's sequence from the passed seq also
    _room.sequence(seq);
}

message *
message_room::duplicate() {
    return new message_room(id(), _room, sender_id(), sequence(), group_base());
}
