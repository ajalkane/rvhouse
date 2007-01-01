#ifndef _MESSAGING_MESSAGE_BLOCK_USERS_H_
#define _MESSAGING_MESSAGE_BLOCK_USERS_H_

#include <string>
#include <list>

#include <netcomgrp/common.h>

#include "message_user.h"

class message_block_users : public message_user {
public:     
    typedef std::list<netcomgrp::uint32_t> list_type;
    
    message_block_users(int msg_type, const chat_gaming::user &u,
                        unsigned seq, int group_base);
    virtual message *duplicate();
        
    inline const void ip_push_back(netcomgrp::uint32_t ip) {
        _ips.push_back(ip);
    }
    inline size_t ip_size() const {
        return _ips.size();
    }
    inline list_type::const_iterator ip_begin() const {
        return _ips.begin();
    }
    inline list_type::const_iterator ip_end() const {
        return _ips.end();
    }
    
private:
    list_type _ips;
};

#endif //_MESSAGING_MESSAGE_BLOCK_USERS_H_
