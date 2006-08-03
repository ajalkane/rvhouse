#include <netcomgrp/exception.h>

#include "../global.h"
#include "../reporter/client.h"
#include "group_adapter_base.h"

namespace networking {

group_adapter_base::group_adapter_base(
    ACE_Reactor *r
) : group_adapter(r), 
    _group(NULL), _handler(NULL), _state(disconnected)
{
}

void
group_adapter_base::init() {
    ACE_DEBUG((LM_DEBUG, "group_adapter_base::init\n"));
    group_adapter::init();
    ACE_DEBUG((LM_DEBUG, "group_adapter_base::init create_group\n"));
    _group   = create_group();
    ACE_DEBUG((LM_DEBUG, "group_adapter_base::init create_handler\n"));
    _handler = create_handler(_group);
    ACE_DEBUG((LM_DEBUG, "group_adapter_base::init done\n"));
}

group_adapter_base::~group_adapter_base() {
    delete _group;
    delete _handler;
}

int group_adapter_base::connect(const chat_gaming::user &self) {
    if (_group->in_state() != netcomgrp::group::not_joined) {
        ACE_DEBUG((LM_WARNING, "group_adapter_base: " \
                  "in state (%s) != not_joined, can't connect\n",
                  _group->in_state_str()));
        return -1;
    }
/*  if (_state != disconnected) {
        ACE_DEBUG((LM_WARNING, "group_adapter_base: in state (%d) != disconnected, can't connect\n"));
        return -1;
    } */
    
    ACE_DEBUG((LM_DEBUG, "group_adapter_base::connect for user '%s'\n",
              self.login_id().c_str()));
    ACE_DEBUG((LM_DEBUG, "group_adapter_base::connect handler is %d\n",
              _handler));

    ACE_DEBUG((LM_DEBUG, "TODO debug user id str: %s\n",
               self.id().id_str().c_str()));                      
    _handler->user_self(self);
    
    try {
        ACE_DEBUG((LM_DEBUG, "group_adapter_base::calling join\n"));
        _group->join(join_group().c_str());
    } catch (netcomgrp::exception &e) {
        // This is mainly to catch address resolution errors, for example
        // if the centralized server is dead. But mayhap is useful for other
        // purposes also.
        ACE_DEBUG((LM_DEBUG, "group_adapter_base: netcomgrp exception received: %s\n",
                   e.what()));
    } catch (...) {
        ACE_DEBUG((LM_DEBUG, "group_adapter_base: some exception received???\n"));
        // In all other cases rethrow the error
        throw;
    }
//  _state = connecting;
    return 0;
}

int group_adapter_base::disconnect() {
    if (_group->in_state() == netcomgrp::group::leaving ||
        _group->in_state() == netcomgrp::group::not_joined) 
    {
        ACE_DEBUG((LM_WARNING, "group_adapter_base: " \
                  "in state (%s) == already disconnecting/disconnected\n",
                  _group->in_state_str()));
        return -1;
    }
/*  if (_state == disconnected ||
        _state == disconnecting) {
        ACE_DEBUG((LM_WARNING, "group_adapter_base: in state (%d) == already disconnecting/disconnected\n"));
        return -1;
    }  */
    
    _group->leave();
    // _state = disconnecting;
    return 0;
}

void group_adapter_base::refresh() {
    _handler->refresh();
}

int group_adapter_base::send(
  const std::string &msg, 
  const std::string &channel, unsigned seq)
{
    if (_group->in_state() != netcomgrp::group::joined) {
        ACE_DEBUG((LM_WARNING, "group_adapter_base: " \
                  "in state (%s) != joined, can't send\n",
                  _group->in_state_str()));
        return -1;
    }

    return _handler->send(msg, channel, seq);
}

// TODO most of these, that are redirected to handler, should really be
// be done here... the current way just adds unnecessary new layer.
int  
group_adapter_base::send_room(const std::string &msg, 
                              const chat_gaming::room::id_type &rid, unsigned seq)
{
    if (_group->in_state() != netcomgrp::group::joined) {
        ACE_DEBUG((LM_WARNING, "group_adapter_base: " \
                  "in state (%s) != joined, can't send\n",
                  _group->in_state_str()));
        return -1;
    }

    return _handler->send_room(msg, rid, seq);
}

void
group_adapter_base::update(const chat_gaming::user &u) {
    _handler->update(u);
}
void
group_adapter_base::update(const chat_gaming::room &r) {
    _handler->update(r);
}

void
group_adapter_base::join(const chat_gaming::room &r, unsigned seq) {
    _handler->join(r,seq);
}

void
group_adapter_base::join_rsp(const chat_gaming::user::id_type &uid, int rsp, unsigned seq) {
    _handler->join_rsp(uid, rsp, seq);
}

void
group_adapter_base::room_command(const chat_gaming::room::id_type &rid, 
                                 int command, unsigned seq)
{
    _handler->room_command(rid, command, seq);
}

void 
group_adapter_base::handle_message(message *msg) {
    _handler->handle_message(msg);
}

} // ns networking
