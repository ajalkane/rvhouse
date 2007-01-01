#ifndef _NETWORKING_REPORTER_CLIENT_H_
#define _NETWORKING_REPORTER_CLIENT_H_

#include <string>
#include <sstream>

#include <ace/Reactor.h>
#include <ace/Svc_Handler.h>
#include <ace/Connector.h>
#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Connector.h>
#include <ace/Reactor_Notification_Strategy.h>

#include "../../common.h"
#include "../../chat_gaming/user.h"

namespace networking {
namespace reporter {
    class client {
        chat_gaming::user _self;
    protected:
        ACE_Reactor *_reactor;
        void _send_user_report(const char *code, 
                               const chat_gaming::user &u, int grp);
        void _send_connect_report(const char *code, int grp);
        
        class handler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> {
            typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;
            // std::ostringstream _buffer;
            void _release_blocks();
            client *_owner;
            ACE_Reactor_Notification_Strategy _notifier;
        public:
            enum {
                disconnected = 0,
                connecting,
                connected
            } state;

            handler(ACE_Reactor *r, client *owner);
            handler();
            virtual ~handler();
            void send(const std::string &str);
            // ACE_Svc_Handler interface
            virtual int open(void *p);
            // virtual int handle_input (ACE_HANDLE fd=ACE_INVALID_HANDLE);
            virtual int handle_output(ACE_HANDLE fd=ACE_INVALID_HANDLE);
            virtual int handle_input(ACE_HANDLE fd=ACE_INVALID_HANDLE);
            virtual int handle_close (ACE_HANDLE handle, ACE_Reactor_Mask close_mask);
            virtual int handle_timeout(const ACE_Time_Value &tv, const void *);         
        };
        
        handler *_handler;
        ACE_Connector<handler, ACE_SOCK_CONNECTOR> *_connector;       
    public:     
        client(ACE_Reactor *r = NULL);
        virtual ~client();
        
        inline void user_self(const chat_gaming::user &s) { 
            _self = s; 
            _self.id(s.id());
        }
        void new_user(const chat_gaming::user &u, int grp);
        void user_updated(const chat_gaming::user &oldu, 
                          const chat_gaming::user &newu, int grp);
        void user_removed(const chat_gaming::user &u, int grp);
        void connected(int grp);
        void disconnected(int grp);
        void disabled(int grp);
        void dht_connected();
        void dht_bootstrap();
        void dht_disconnected();
        void dht_ip_found(const ACE_INET_Addr &addr);

        void ext_ip_detected(const std::string &ipstr);
        void ext_ip_failed();
        
        inline void reactor(ACE_Reactor *r) { _reactor = r; }
        inline ACE_Reactor *reactor() const { return _reactor; }                
    };  
} // ns reporter
} // ns networking

#endif //_NETWORKING_REPORTER_CLIENT_H_
