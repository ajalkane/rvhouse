#ifndef _MESSAGING_MESSAGE_STRING_H_
#define _MESSAGING_MESSAGE_STRING_H_

#include <string>

#include "message.h"

class message_string : public message {
    std::string _str;
public: 
    message_string(int msg_type, const std::string &s); 
    virtual message *duplicate();
    
    inline const std::string &str() const;
};

inline const std::string &message_string::str() const { return _str; }

#endif //_MESSAGING_MESSAGE_STRING_H_
