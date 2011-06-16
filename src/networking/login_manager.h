#ifndef _NETWORKING_LOGIN_MANAGER_H_
#define _NETWORKING_LOGIN_MANAGER_H_

#include <string>

#include <ace/Reactor.h>

#include "../messaging/messenger.h"
#include "../config_file.h"
#include "group_handler.h"

namespace networking {
    class login_manager {
    public:
        login_manager(ACE_Reactor *r = NULL);
        virtual ~login_manager();
        
        inline void reactor(ACE_Reactor *r);
        inline ACE_Reactor *reactor() const;
                
        virtual void login        (const message *msg) = 0;
        virtual void register_user(const message *msg) = 0;
    protected:
        ACE_Reactor *_reactor;
    };
    
    inline void
    login_manager::reactor(ACE_Reactor *r) {
        _reactor = r;
    }
    inline ACE_Reactor *
    login_manager::reactor() const { return _reactor; } 
} // ns networking

#endif //_NETWORKING_LOGIN_MANAGER_H_
