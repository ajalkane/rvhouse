#ifndef _NETWORKING_LOGIN_MANAGER_RVZT_H_
#define _NETWORKING_LOGIN_MANAGER_RVZT_H_

#include <string>

#include <fx.h>
#include <ace/Reactor.h>

#include "../login_manager.h"
#include "../http/fetcher.h"

namespace networking {
    // Implementation of the login manager that uses RVZT's
    // database. Should move to public key cryptography and
    // to using some key server, but this will do as 
    // a starting point.
    class login_manager_rvzt 
        : public login_manager {
    public:
        login_manager_rvzt(ACE_Reactor *r = NULL);
        virtual ~login_manager_rvzt();
                
        virtual void login        (const message *msg);
        virtual void register_user(const message *msg);
    private:
        http::fetcher *_http_fetcher;
        std::string    _base_url;

        void _login_operation(const std::string &req_url, 
                              const std::string &user,
                              int _msg_base);
        
        class _fetch_handler : public http::handler {
            login_manager_rvzt *_owner;
            std::string         _user;
            int                 _msg_base;
        public:
            _fetch_handler(login_manager_rvzt *o,
                           const std::string &user,
                           int msg_base) 
              : _owner(o), _user(user), _msg_base(msg_base) {}
            // http::handler interface      
            virtual int handle_response(const http::response &resp);
            virtual int handle_error(int reason, const char *details);           
            virtual int handle_close();
        };
        
    };  
} // ns networking

#endif //_NETWORKING_LOGIN_MANAGER_RVZT_H_
