#ifndef _MESSAGING_MESSAGE_USER_H_
#define _MESSAGING_MESSAGE_USER_H_

#include "../chat_gaming/user.h"

#include "message.h"
#include "message_grouped.h"

class message_user : public message_grouped {
    chat_gaming::user _user;
public: 
    message_user(int msg_type, const chat_gaming::user &u, 
                 unsigned seq, int group_base); 
    virtual message *duplicate();
    
    inline const chat_gaming::user &user() const;
};

inline const chat_gaming::user &
message_user::user() const { return _user; }

#endif //_MESSAGING_MESSAGE_USER_H_
