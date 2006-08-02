#ifndef _MESSAGING_MESSAGE_REGISTER_H_
#define _MESSAGING_MESSAGE_REGISTER_H_

#include <string>

#include "message_login.h"

class message_register : public message_login {
public:
    message_register(int msg_type,
                     const std::string &u,
                     const std::string &p,
                     const std::string &m);
    virtual message *duplicate();

    inline const std::string &mail() const;
private:
    std::string _mail;
};

inline const std::string &
message_register::mail() const { return _mail; }

#endif //_MESSAGING_MESSAGE_REGISTER_H_
