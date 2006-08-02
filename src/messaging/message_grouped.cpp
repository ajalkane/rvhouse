#include "message_grouped.h"

message_grouped::message_grouped(
    int t, const chat_gaming::user::id_type &sid, unsigned s, int gb)
        : message(t), _sender_id(sid), _seq(s), _group_base(gb) {}

message *
message_grouped::duplicate() {
    return new message_grouped(id(), _sender_id, _seq, _group_base);
}
