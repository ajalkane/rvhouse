#ifndef _MESSAGING_MESSAGE_GLOBAL_IGNORE_H_
#define _MESSAGING_MESSAGE_GLOBAL_IGNORE_H_

#include <string>
#include <list>

#include "message.h"

class message_global_ignore : public message {
public:
    struct ip_entry {
        std::string ip;
        std::string userid;
        std::string reason;
    };
    
    typedef std::list<ip_entry> ip_list_type;
    
    message_global_ignore(int msg_type);  
    virtual message *duplicate();
        
    inline const void ip_push_back(const ip_entry &e) {
        _ip_list.push_back(e);
    }
    inline size_t ip_size() const {
        return _ip_list.size();
    }
    inline ip_list_type::const_iterator ip_begin() const {
        return _ip_list.begin();
    }
    inline ip_list_type::const_iterator ip_end() const {
        return _ip_list.end();
    }
    
    inline size_t size() const {
        return ip_size();
    }
private:    
    ip_list_type _ip_list;
};

#endif //_MESSAGING_MESSAGE_GLOBAL_IGNORE_H_
