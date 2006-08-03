#include "message_channel.h"

message_channel::message_channel(
    int t, 
    const std::string &str,
    const chat_gaming::user::id_type &uid, 
    const std::string &channel,
    unsigned seq,
    int gb
) : message_send(t, str, uid, seq, gb), _channel(channel) {}

message *
message_channel::duplicate() {
    return new message_channel(id(), str(), sender_id(), channel(), sequence(), group_base());
}
