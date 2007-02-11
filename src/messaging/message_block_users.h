#ifndef _MESSAGING_MESSAGE_BLOCK_USERS_H_
#define _MESSAGING_MESSAGE_BLOCK_USERS_H_

#include <string>
#include <list>

#include <netcomgrp/common.h>

#include "message_user.h"

class message_block_users : public message_user {
public:
    struct ipandmask {
        netcomgrp::uint32_t ip;
        netcomgrp::uint32_t mask;
        inline ipandmask(
            netcomgrp::uint32_t i = INADDR_NONE, 
            netcomgrp::uint32_t m = INADDR_NONE
        ) : ip(i), mask(m) {}
    };
    
    typedef std::list<ipandmask> list_type;
    
    message_block_users(int msg_type, const chat_gaming::user &u,
                        unsigned seq, int group_base);
    virtual message *duplicate();
    
    inline bool global_ignore() const { return _global_ignore;     }
    inline bool global_ignore(bool g) { return _global_ignore = g; }
    
    inline const void ip_push_back(
        netcomgrp::uint32_t ip,
        netcomgrp::uint32_t mask = INADDR_NONE
    ) {
        _ips.push_back(ipandmask(ip, mask));
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
    bool      _global_ignore;
};

#endif //_MESSAGING_MESSAGE_BLOCK_USERS_H_
