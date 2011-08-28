#ifndef _NETWORKING_REPORTER_CLIENT_INTERFACE_H_
#define _NETWORKING_REPORTER_CLIENT_INTERFACE_H_

#include <string>
#include <sstream>

#include <ace/SOCK_Stream.h>

#include "../../common.h"
#include "../../chat_gaming/user.h"

namespace networking {
namespace reporter {
    class client_interface {
    public:
        virtual ~client_interface();
        virtual void user_self(const chat_gaming::user &s) = 0;
        virtual void new_user(const chat_gaming::user &u, int grp) = 0;
        virtual void user_updated(const chat_gaming::user &oldu,
                          const chat_gaming::user &newu, int grp) = 0;
        virtual void user_removed(const chat_gaming::user &u, int grp) = 0;
        virtual void connected(int grp) = 0;
        virtual void disconnected(int grp) = 0;
        virtual void disabled(int grp) = 0;
        virtual void dht_connected() = 0;
        virtual void dht_bootstrap() = 0;
        virtual void dht_disconnected() = 0;
        virtual void dht_ip_found(const ACE_INET_Addr &addr) = 0;

        virtual void ext_ip_detected(const std::string &ipstr) = 0;
        virtual void ext_ip_failed() = 0;
    };  
} // ns reporter
} // ns networking

#endif //_NETWORKING_REPORTER_CLIENT_INTERFACE_H_
