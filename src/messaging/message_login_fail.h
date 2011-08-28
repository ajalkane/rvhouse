#ifndef _MESSAGING_MESSAGE_LOGIN_FAIL_H_
#define _MESSAGING_MESSAGE_LOGIN_FAIL_H_

#include <string>

#include "message.h"

class message_login_fail : public message {
public:     
    message_login_fail(int msg_type, const std::string &status);
    virtual message *duplicate();
    
    inline const std::string &status() const;
private:
    std::string _status;
};

inline const std::string &
message_login_fail::status() const { return _status; }

#endif //_MESSAGING_MESSAGE_LOGIN_FAIL_H_
