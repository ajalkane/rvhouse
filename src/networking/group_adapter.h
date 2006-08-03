#ifndef _NETWORKING_GROUP_ADAPTER_H_
#define _NETWORKING_GROUP_ADAPTER_H_

#include <string>

#include <fx.h>
#include <ace/Reactor.h>

#include "../chat_gaming/user.h"
#include "../chat_gaming/room.h"
#include "../messaging/messenger.h"
#include "../config_file.h"
#include "group_handler.h"

namespace networking {
    class group_adapter {
    protected:
        ACE_Reactor *_reactor;
    public:
        enum {
            disconnected,
            connecting,
            connected,
            disconnecting,
            
            updating,
            
            last
        };
        
        group_adapter(ACE_Reactor *r = NULL);
        virtual ~group_adapter();
        
        inline void reactor(ACE_Reactor *r);
        inline ACE_Reactor *reactor() const;
                
        virtual void init();
        virtual int  connect(const chat_gaming::user &self) = 0;
        virtual int  disconnect() = 0;
        virtual void refresh() = 0;
        
        virtual int  send(const std::string &msg, const std::string &channel, unsigned seq) = 0;
        virtual int  send_room(const std::string &msg, 
                               const chat_gaming::room::id_type &rid, unsigned seq) = 0;
        virtual void update_server_nodes() = 0;     
        
        virtual void update(const chat_gaming::user &u) = 0;
        virtual void update(const chat_gaming::room &r) = 0;
        virtual void join  (const chat_gaming::room &r, unsigned seq) = 0;
        virtual void join_rsp(const chat_gaming::user::id_type &uid, int rsp_code, unsigned seq) = 0;

        virtual void room_command(const chat_gaming::room::id_type &rid, 
                                  int command, unsigned seq) = 0;
                                  
        virtual void handle_message(message *msg) = 0;
    };
    
    inline void
    group_adapter::reactor(ACE_Reactor *r) {
        _reactor = r;
    }
    inline ACE_Reactor *
    group_adapter::reactor() const { return _reactor; }
    
} // ns networking

#endif //_NETWORKING_GROUP_ADAPTER_H_
