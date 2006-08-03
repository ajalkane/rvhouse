#ifndef _MESSAGING_MESSAGE_SEND_H_
#define _MESSAGING_MESSAGE_SEND_H_

#include <string>

#include "../chat_gaming/user.h"

#include "message_grouped.h"

class message_send : public message_grouped {
    std::string _str;
public: 
    message_send(int msg_type, 
                 const std::string &str,
                 const chat_gaming::user::id_type &uid, 
                 unsigned seq,
                 int group_base);
                 
    virtual message *duplicate();
    
    inline const std::string &str() const {
        return _str;
    }
};

#endif //_MESSAGING_MESSAGE_SEND_H_
