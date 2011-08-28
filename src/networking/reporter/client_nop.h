#ifndef _NETWORKING_REPORTER_CLIENT_NOP_H_
#define _NETWORKING_REPORTER_CLIENT_NOP_H_

#include "client_interface.h"

#include "../../common.h"
#include "../../chat_gaming/user.h"

/**
 * Implements reporting client no-operation, ie. nothing is sent and no connection made
 */
namespace networking {
namespace reporter {
    class client_nop : public client_interface {
    public:     
        client_nop();
        virtual ~client_nop();
        
        void user_self(const chat_gaming::user &s);
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
    };  
} // ns reporter
} // ns networking

#endif //_NETWORKING_REPORTER_CLIENT_NOP_H_
