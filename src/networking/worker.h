#ifndef _NETWORKING_WORKER_H_
#define _NETWORKING_WORKER_H_

#include <string>

#include <ace/Reactor.h>
#include <ace/Task.h>
#include <ace/Event_Handler.h>

#include <dht/client.h>
#include <netcomgrp/group.h>

#include "../messaging/messenger.h"
#include "../messaging/message.h"
#include "../config_memory.h"

#include "version_update/client.h"
#include "global_ignore/client.h"
#include "group_adapter.h"
#include "login_manager.h"

namespace networking {
    class worker : public ACE_Task_Base {
        group_adapter    *_group_adapter;
        ACE_Reactor      *_reactor;
        
        login_manager    *_login_manager;
        std::string       _exception_str;
        version_update::client *_version_update_client;
        global_ignore::client  *_global_ignore_client;
        bool _quit;
        
        void _main();
    protected:
        // FXint run();
        virtual int svc();
        void handle_message(message *msg);
    public:
        worker();
        virtual ~worker();
            
        // ACE_Event_Handler interface
        virtual int handle_exception(ACE_HANDLE);
    };
} // ns networking

#endif //_WORKER_H_
