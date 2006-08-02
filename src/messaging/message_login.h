#ifndef _MESSAGING_MESSAGE_LOGIN_H_
#define _MESSAGING_MESSAGE_LOGIN_H_

#include <string>

#include "message.h"

class message_login : public message {
public:
    message_login(int msg_type, const std::string &u, const std::string &p);
    virtual message *duplicate();

    inline const std::string &user() const;
    inline const std::string &pass() const;
private:
    std::string _user;
    std::string _pass;
};

inline const std::string &
message_login::user() const { return _user; }
inline const std::string &
message_login::pass() const { return _pass; }

#endif //_MESSAGING_MESSAGE_LOGIN_H_
