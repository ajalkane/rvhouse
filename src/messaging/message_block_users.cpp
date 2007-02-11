#include "message_block_users.h"

message_block_users::message_block_users(
    int t, const chat_gaming::user &u, 
    unsigned seq, int gb
) : message_user(t, u, seq, gb), _global_ignore(false) {}

message *
message_block_users::duplicate() {
    return new message_block_users(*this);
}
