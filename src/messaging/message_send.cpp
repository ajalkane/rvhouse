#include "message_send.h"

message_send::message_send(
    int t,
    const std::string &str,
    const chat_gaming::user::id_type &uid,
    unsigned seq,
    int gb
) : message_grouped(t, uid, seq, gb), _str(str)
{
}

message *
message_send::duplicate() {
    return new message_send(id(),  str(), sender_id(), sequence(), group_base());
}
