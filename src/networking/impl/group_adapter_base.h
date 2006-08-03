#ifndef _NETWORKING_IMPL_GROUP_ADAPTER_BASE_H_
#define _NETWORKING_IMPL_GROUP_ADAPTER_BASE_H_

#include <string>

#include "../group_adapter.h"
#include "group_handler_base.h"

namespace networking {
    class group_adapter_base : public group_adapter {
    protected:
        netcomgrp::group   *_group;  // Must be set by derived classes
        group_handler_base *_handler;
        
        int _state;
        
        virtual netcomgrp::group  *create_group() = 0;
        virtual const std::string &join_group()   = 0;
        
        virtual group_handler_base *create_handler(netcomgrp::group *g) = 0;
    public:
        group_adapter_base(ACE_Reactor *r = NULL);
        virtual ~group_adapter_base();

        virtual void init();                
        virtual int connect(const chat_gaming::user &self);
        virtual int disconnect();
        virtual void refresh();
        
        virtual int send(const std::string &msg, const std::string &channel, unsigned seq);
        virtual int send_room(const std::string &msg, 
                              const chat_gaming::room::id_type &rid, unsigned seq);     
        virtual void update_server_nodes() {}
        virtual void update(const chat_gaming::user &u);
        virtual void update(const chat_gaming::room &r);
        virtual void join  (const chat_gaming::room &r, unsigned seq);
        virtual void join_rsp(const chat_gaming::user::id_type &uid, int rsp_code, unsigned seq);

        virtual void room_command(const chat_gaming::room::id_type &rid, 
                                  int command, unsigned seq);

        virtual void handle_message(message *msg);                       
        
    };  
} // ns networking

#endif //_NETWORKING_IMPL_GROUP_ADAPTER_BASE_H_
