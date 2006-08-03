#include "message_user.h"

message_user::message_user(int t, const chat_gaming::user &u, 
unsigned seq,int gb) 
  : message_grouped(t, u.id(), seq, gb), _user(u) 
{
    // Set user's sequence from the passed seq also
    _user.sequence(seq);  
}

message *
message_user::duplicate() {
    return new message_user(id(), _user, sequence(), group_base());
}
