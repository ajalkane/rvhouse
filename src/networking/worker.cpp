#include <algorithm>
#include <exception>
#include <utility>

#include <ace/ACE.h>
#include <ace/Thread.h>
#include <ace/Log_Msg.h>
#include <ace/Reactor.h>

#include <reudp/config.h>

#include "worker.h"
#include "global.h"

#include "../util.h"
#include "../config_file.h"
#include "../messaging/message.h"
#include "../messaging/message_string.h"
#include "../messaging/message_user.h"
#include "../messaging/message_channel.h"
#include "../messaging/message_send_room.h"
#include "../messaging/message_room.h"
#include "../messaging/message_room_join.h"
#include "../messaging/message_room_command.h"
#include "../messaging/impl/ace_messenger.h"
#include "../chat_gaming/pdu/header.h"

#include "reporter/client.h"
#include "impl/group_adapter_combine.h"
#include "impl/login_manager_rvzt.h"
#include "ip_block/store.h"

namespace networking {

worker::worker()
    : _quit(false)
{
    _reactor   = ACE_Reactor::instance();
    net_messenger.instance(new ace_messenger(_reactor, this));
    net_conf.instance(new config_file(*(conf())));
    net_ip_block.instance(new ip_block::store);
    _version_update_client = NULL;
}

worker::~worker() {
    delete net_messenger();
    delete net_conf();
    delete net_ip_block();
}

int
worker::svc() {
    ACE_DEBUG((LM_DEBUG, "networking::worker thread started\n"));
    try {
        _main();
    } catch (const std::exception &e) {
        ACE_DEBUG((LM_ERROR, "networking::worker dying of exception %s\n",
                   e.what()));;
        message_string *m = 
          new message_string(message::worker_exception, e.what());
        gui_messenger()->send_msg(m);
    } catch (...) {
        ACE_DEBUG((LM_ERROR, "some unrecognized exception received, dying!\n"));
    }
    
    delete net_report(); net_report.instance(NULL);
    
    ACE_DEBUG((LM_DEBUG, "networking::worker thread exit\n"));
    return 0;
}

void
worker::_main() {
    // Configure reudp if need to
    {
        size_t tout_msec = net_conf()->get<size_t>("net_reudp", "timeout", 0);
        size_t send_amnt = net_conf()->get<size_t>("net_reudp", "send_try_count", 0);
        
        if (tout_msec != 0) {
            reudp::time_value_type tout;
            tout.msec(tout_msec);
            reudp::config::timeout(tout);
        }
        if (send_amnt != 0) {
            reudp::config::send_try_count(send_amnt);
        }
    }
            
    // Networking worker thread must own the reactor so that it can be 
    // signalled to wake up
    _reactor->owner(ACE_Thread::self());
    net_report.instance(new reporter::client(_reactor));
    
    _group_adapter = 
        new group_adapter_combine(_reactor);
    std::auto_ptr<group_adapter> ga_guard(_group_adapter);
    
    _login_manager = new login_manager_rvzt(_reactor);
    std::auto_ptr<login_manager> lm_guard(_login_manager);
    
    _group_adapter->init();
    
    _version_update_client = new version_update::client;
    
    while (!_quit) {
        ACE_DEBUG((LM_DEBUG, "worker: handle_events\n"));
        // TODO check error, throw exception
        _reactor->handle_events();
    }
    
    if (!_exception_str.empty()) {
        ACE_DEBUG((LM_DEBUG, "worker::exception string set, throwing: %s\n",
                   _exception_str.c_str()));
        // Work around for ACE peculiarity
        throw exception(0, _exception_str.c_str());
    }
    
    delete _version_update_client;
}

int
worker::handle_exception(ACE_HANDLE) {
    ACE_DEBUG((LM_DEBUG, "networking::worker: handle_messenger received at thread %t\n"));
        
    typedef std::list<message *> ctype;
    ctype msgs;
    std::insert_iterator<ctype> inserter(msgs, msgs.begin());
    
    net_messenger()->collect_msgs(inserter);

    try {   
        ctype::iterator i = msgs.begin();
        for (; i != msgs.end(); i++) {
            ACE_DEBUG((LM_DEBUG, "networking::worker: msg is %d\n", (*i)->id()));
            
            // At least for now, just pass the message to house_window for handling
            this->handle_message(*i);
        }
    } catch (const std::exception &e) {
        ACE_DEBUG((LM_DEBUG, "worker::handle_exception: gots exception uh\n"));
        
        // ACE has some strange troubles with rethrowing the exception from
        // here... the exception does not end up being catched by worker's
        // main, but in the real program main! Yet in the worker thread.
        // Peculiar, but no time to look into it at the moment, so do work
        // around it like this:
        _quit = true;
        _exception_str = e.what();
        // Ensure all messages are deleted even in case of an error
        // std::for_each(msgs.begin(), msgs.end(), delete_ptr<message>());
        // ACE_DEBUG((LM_DEBUG, "worker::handle_exception: rehtrowing\n"));
        // throw;
    }
    std::for_each(msgs.begin(), msgs.end(), delete_ptr<message>());

    ACE_DEBUG((LM_DEBUG, "networking::worker: handle_messenger exiting\n"));
    
    return 1;   
}

void
worker::handle_message(message *msg) {
    switch (msg->id()) {
    case message::connect:
    {
        ACE_DEBUG((LM_DEBUG, "networking::worker: connect received\n"));
        message_user *m = dynamic_ptr_cast<message_user>(msg);
        ACE_DEBUG((LM_DEBUG, "TODO debug user id str: %s\n",
                  m->user().id().id_str().c_str()));        
        _group_adapter->connect(m->user());
    }
        break;
    case message::disconnect:
        ACE_DEBUG((LM_DEBUG, "networking::worker: disconnect received\n"));
        _group_adapter->disconnect();
        break;
    case message::version_fetch:
        ACE_DEBUG((LM_DEBUG, "networking::worker: version_fetch received\n"));
        // Deletes itself automatically after fetch has been called
        _version_update_client->fetch();
        _version_update_client = NULL;
        break;
    case message::send:
    {
        message_send *m = dynamic_ptr_cast<message_send>(msg);
        ACE_DEBUG((LM_DEBUG, "networking::worker: send received\n"));
        // TODO modify send to be without channel - this is a public
        // message sent to all
        _group_adapter->send(m->str(), std::string(), m->sequence());
    }
        break;
    case message::send_room:
    {
        message_send_room *m = dynamic_ptr_cast<message_send_room>(msg);
        ACE_DEBUG((LM_DEBUG, "networking::worker: send received\n"));
        _group_adapter->send_room(m->str(), m->room_id(), m->sequence());
    }
        break;
    case message::exit_now:
        ACE_DEBUG((LM_DEBUG, "networking::worker: exit now received\n"));
        _quit = true;
        break;
    case message::login:
        ACE_DEBUG((LM_DEBUG, "networking::worker: user login\n"));
        _login_manager->login(msg);
        break;
    case message::refresh:
        ACE_DEBUG((LM_DEBUG, "networking::worker: refresh\n"));
        _group_adapter->refresh();
        break;
    case message::reg:
        ACE_DEBUG((LM_DEBUG, "networking::worker: user register\n"));
        _login_manager->register_user(msg);
        break;
/*  case message::send_data:
        break; */
    case message::user:
    {
        ACE_DEBUG((LM_DEBUG, "networking::worker: user update received\n"));
        message_user *m = dynamic_ptr_cast<message_user>(msg);
        _group_adapter->update(m->user());
    }
        break;
    case message::block_users:
        net_ip_block()->handle_message(msg);
        _group_adapter->handle_message(msg);
        break;
    case message::room:
    {
        ACE_DEBUG((LM_DEBUG, "networking::worker: room update received\n"));
        message_room *m = dynamic_ptr_cast<message_room>(msg);
        _group_adapter->update(m->room());
    }
        break;
    case message::room_launch:
#if 0
    {
        ACE_DEBUG((LM_DEBUG, "networking::worker: room launch received\n"));
        message_room_command *m = dynamic_ptr_cast<message_room_command>(msg);
        _group_adapter->room_command(m->room_id(), chat_gaming::pdu::id_room_launch,
                                     m->sequence());
    }
#endif
        _group_adapter->handle_message(msg);
        break;
    case message::room_kick:
    case message::send_private:
    case message::private_refused:
        _group_adapter->handle_message(msg);
        break;
    case message::room_join:
    {
        ACE_DEBUG((LM_DEBUG, "networking::worker: room update received\n"));
        message_room *m = dynamic_ptr_cast<message_room>(msg);
        _group_adapter->join(m->room(), m->sequence());
    }
        break;
    case message::room_join_pass:
    {
        ACE_DEBUG((LM_DEBUG, "networking::worker: room join pass response received\n"));
        message_room_join *m = dynamic_ptr_cast<message_room_join>(msg);
        _group_adapter->join_rsp(m->sender_id(), chat_gaming::pdu::id_room_join_password,
                                 m->sequence());
    }
        break;
    case message::room_join_full:
    {
        ACE_DEBUG((LM_DEBUG, "networking::worker: room join full response received\n"));
        message_room_join *m = dynamic_ptr_cast<message_room_join>(msg);
        _group_adapter->join_rsp(m->sender_id(), chat_gaming::pdu::id_room_join_full,
                                 m->sequence());
    }
        break;
    case message::room_join_ack:
    {
        ACE_DEBUG((LM_DEBUG, "networking::worker: room join ack response received\n"));
        message_room_join *m = dynamic_ptr_cast<message_room_join>(msg);
        _group_adapter->join_rsp(m->sender_id(), chat_gaming::pdu::id_room_join_ack,
                                 m->sequence());
    }
        break;
    default:
        ACE_DEBUG((LM_DEBUG, "networking::worker: unrecognized message %d " \
                   "recceived\n", msg->id()));
    }
}


} // ns networking
