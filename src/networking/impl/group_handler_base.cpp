#include <vector>

#include "group_handler_base.h"
#include "../../util.h"
#include "../../chat_gaming/pdu/header.h"
#include "../../chat_gaming/pdu/message.h"
#include "../../chat_gaming/pdu/room_join.h"
#include "../../chat_gaming/pdu/room_command.h"
#include "../../messaging/message_user.h"
#include "../../messaging/message_room.h"
#include "../../messaging/message_room_join.h"
#include "../../messaging/message_room_command.h"
#include "../../messaging/message_channel.h"
#include "../../messaging/message_send.h"
#include "../../messaging/message_block_users.h"
#include "../worker.h"
#include "../global.h"
#include "../reporter/client.h"
#include "../ip_block/store.h"

namespace networking {

#define GROUP_MESSAGE(msg) (msg - message::group_base + this->_grp_msg_base)

void group_handler_notify::group_state_changed(int state) {}
void group_handler_notify::group_error(int err, const char *desc) {}

group_handler_base::group_handler_base(
  netcomgrp::group *grp, int msgbase
  ) : _grp(grp), _grp_msg_base(msgbase), _notify(NULL)
{
    _grp->observer_attach(this, 
                          netcomgrp::event_observer::mask_data_received |
                          netcomgrp::event_observer::mask_node_added    |
                          netcomgrp::event_observer::mask_node_removed  |
                          netcomgrp::event_observer::mask_state_changed |
                          netcomgrp::event_observer::mask_error);
                          
    _house_adapter = new _house_adapter_type(grp, &_house, this);
}

group_handler_base::~group_handler_base() {}

void 
group_handler_base::handle_message(message *msg) {
    switch (msg->id()) {
    case message::room_kick:
    case message::room_launch:
        handle_room_command_message(msg);
        break;
    case message::send_private:
        handle_send_private_message(msg);
        break;
    case message::private_refused:
        handle_private_refused(msg);
        break;
    case message::block_users:
        handle_block_users(msg);
        break;
    }   
}

void 
group_handler_base::handle_room_command_message(message *msg) {
    message_room_command *m = dynamic_ptr_cast<message_room_command>(msg);
    int pdu_cmd = -1;
    switch(m->id()) {
    case message::room_kick:   pdu_cmd = chat_gaming::pdu::id_room_kick;   break;
    case message::room_launch: pdu_cmd = chat_gaming::pdu::id_room_launch; break;
    default:
        ACE_ERROR((LM_ERROR, "Invalid room command %d\n", m->id()));
        return;
    }
    
    _house_adapter->send_to_room(
        chat_gaming::pdu::header(pdu_cmd, m->sequence()),
        chat_gaming::pdu::room_command(m->room_id(), m->target_user_id()),
        m->room_id()
    );
}

void
group_handler_base::handle_send_private_message(message *msg) {
    message_channel *m         = dynamic_ptr_cast<message_channel>(msg);
    unsigned int seq           = m->sequence();
    const std::string &str     = m->str();
    const std::string &channel = m->channel();
    
    const chat_gaming::house::user_type::id_type &uid = m->sender_id();
    chat_gaming::house::user_iterator ui = _house.user_find(uid.id_str());

    // Send the private message to recipient and self (for echo)
    std::vector<const netcomgrp::node *> user_nodes(2);
    std::vector<const netcomgrp::node *>::const_iterator un_i = user_nodes.begin();
    if (ui == _house.user_end()) {
        // Not really a bad error since the message can be sent via
        // several sources (DHT and centralized) and the user might be
        // associated with one source only. So this will return an error
        // when it is routed to the source that it didn't originate from.
        // But the other branch where it goes won't give error.
        ACE_ERROR((LM_WARNING, "group_handler_base::send_private_message: no user "
                  "found for id %s group %d\n", uid.c_str(), _grp_msg_base));
        user_nodes[0] = NULL;
    } else user_nodes[0] = ui->id().node();

    user_nodes[1] = _grp->self();
    
    for (; un_i != user_nodes.end(); un_i++) {
        ACE_DEBUG((LM_DEBUG, "group_handler_base::send_private_message sending to node ptr %d\n",
                   *un_i));

        if (*un_i) {
            _house_adapter->send_to(
                chat_gaming::pdu::header(chat_gaming::pdu::id_message_private, seq),
                chat_gaming::pdu::message(str, channel),
                *un_i
            );
        }       
    }
}

void
group_handler_base::handle_private_refused(message *msg) {
    message_channel *m         = dynamic_ptr_cast<message_channel>(msg);
    unsigned int seq           = m->sequence();
    const std::string &str     = m->str();
    const std::string &channel = m->channel();
    
    const chat_gaming::house::user_type::id_type &uid = m->sender_id();
    chat_gaming::house::user_iterator ui = _house.user_find(uid.id_str());

    if (ui == _house.user_end()) {
        // Not really a bad error since the message can be sent via
        // several sources (DHT and centralized) and the user might be
        // associated with one source only. So this will return an error
        // when it is routed to the source that it didn't originate from.
        // But the other branch where it goes won't give error.
        ACE_ERROR((LM_WARNING, "group_handler_base::send_private_message: no user "
                  "found for id %s group %d\n", uid.c_str(), _grp_msg_base));
        return;
    }

    ACE_DEBUG((LM_DEBUG, "group_handler_base::private_refused sending to node ptr %d\n",
               ui->node()));

    _house_adapter->send_to(
        chat_gaming::pdu::header(chat_gaming::pdu::id_private_refused, seq),
        chat_gaming::pdu::message(str, channel),
        ui->node()
    );
}

void
group_handler_base::handle_block_users(message *msg) {
    message_block_users *m  = dynamic_ptr_cast<message_block_users>(msg);
#if 0
      
    message_block_users::list_type::const_iterator i   = m->ip_begin();
    message_block_users::list_type::const_iterator end = m->ip_end();
    for (; i != end; i++) {
        ip_blocked(i->ip, i->mask, m->sequence());
    }
#endif
    remove_blocked_users(m->sequence());
}

void
group_handler_base::remove_blocked_users(unsigned seq) {    
    // Find all nodes with a blocked IP and do node_removed for them.
    _house_type::user_iterator i   = _house.user_begin();
    _house_type::user_iterator end = _house.user_end();
    typedef std::list<const netcomgrp::node *> list_type;
    list_type rm;
    
    for (; i != end; i++) {
        if (i->node() == NULL) continue;
        if (net_ip_block()->is_blocked(i->node()->addr().get_ip_address()))
            rm.push_back(i->node());
    }
    list_type::const_iterator ri = rm.begin();
    for (; ri != rm.end(); ri++) {
        ACE_DEBUG((LM_DEBUG, "group_handler_base::remove_blocked_users removing node "
                  "%s:%d\n", 
                  (*ri)->addr().get_host_addr(), 
                  (*ri)->addr().get_port_number()));

        _house_adapter->send_to(
            chat_gaming::pdu::header(chat_gaming::pdu::id_user_blocked, seq),
            *ri
        );
                  
        node_removed(*ri);
    }
}

#if 0    
void
group_handler_base::ip_blocked(uint32_t ip, uint32_t mask, unsigned seq) {
    char ip_str[INET_ADDRSTRLEN];
    if (ACE_OS::inet_htop(AF_INET, &ip, ip_str, sizeof(ip_str))) {
        ACE_DEBUG((LM_DEBUG, "group_handler_base::ip_blocked %s\n", ip_str));
    } else {
        ACE_ERROR((LM_ERROR, "group_handler_base::ip_blocked conversion failed\n"));
    }
    
    // Find all nodes with the blocked IP and do node_removed for them.
    _house_type::user_iterator i   = _house.user_begin();
    _house_type::user_iterator end = _house.user_end();
    typedef std::list<const netcomgrp::node *> list_type;
    list_type rm;
    
    for (; i != end; i++) {
        if (i->node() == NULL) continue;
        uint32_t ip2 = htonl(i->node()->addr().get_ip_address());
        char ip_str2[INET_ADDRSTRLEN];
        ACE_OS::inet_ntop(AF_INET, &ip2, ip_str2, sizeof(ip_str2));
        ACE_DEBUG((LM_DEBUG, "ip_blocked: comparing %s to %s\n", ip_str, ip_str2));
        if (ip2 == ip)
            rm.push_back(i->node());
    }
    list_type::const_iterator ri = rm.begin();
    for (; ri != rm.end(); ri++) {
        ACE_DEBUG((LM_DEBUG, "group_handler_base::ip_blocked removing node "
                  "%s:%d\n", 
                  (*ri)->addr().get_host_addr(), 
                  (*ri)->addr().get_port_number()));

        _house_adapter->send_to(
            chat_gaming::pdu::header(chat_gaming::pdu::id_user_blocked, seq),
            *ri
        );
                  
        node_removed(*ri);
    }
}
#endif

int
group_handler_base::node_added(const netcomgrp::node *n) {
    ACE_DEBUG((LM_DEBUG, "Checking if it is self... self is %d, node %d\n",
              _grp->self(), n));

    ACE_DEBUG((LM_DEBUG, "addr: %s:%d\n", 
              n->addr().get_host_addr(),
              n->addr().get_port_number()));

    if (net_ip_block()->is_blocked(n->addr().get_ip_address())) {
        ACE_DEBUG((LM_DEBUG, "group_handler_base: node added from blocked IP %s "
                  "received", n->addr().get_host_addr()));
        return 0;
    }
              
    _house_adapter->node_added(n);
    
    ACE_DEBUG((LM_DEBUG, "group_handler_base::node added, checking if self, "
              "self/node: %d/%d\n", _grp->self(), n));
    
    if (_grp->self() && *(_grp->self()) == *n) {
        chat_gaming::user &self_ref = _house_adapter->self_user();
        const chat_gaming::user &s = _house_adapter->self_user();
        ACE_DEBUG((LM_DEBUG, "group_handler_base::node_added user self now %s (room_id %s) grp %d\n",
                  s.id().c_str(), s.room_id().c_str(), _grp_msg_base));
        
        ACE_DEBUG((LM_DEBUG, "group_handler_base: user self found, setting " \
                  "user login to %s\n", self_ref.login_id().c_str()));
                  
        _house_type::user_iterator ui = _house.user_find(_user_type(n));
        if (ui == _house.user_end()) {
            ACE_ERROR((LM_ERROR, "group_handler_base: user self, not found " \
                      "from house\n"));
            return 0;
        }

        user_update(*ui, self_ref, n);

        ACE_DEBUG((LM_DEBUG, "group_handler_base::node_added user self2 now %s (room_id %s) grp %d\n",
                  s.id().c_str(), s.room_id().c_str(), _grp_msg_base));

        *ui = self_ref; // _house_adapter->self_user(); // _self;
        // Can set the id string after having added the user since the 
        // actual used sorting key is the node associated with self
        chat_gaming::user_key new_key(ui->id());
        new_key.id_str(self_ref.id().id_str());
        ui->id(new_key);
        _house_adapter->self_user().id(new_key);

        ACE_DEBUG((LM_DEBUG, "group_handler_base::node_added user self3 now %s (room_id %s) grp %d\n",
                  s.id().c_str(), s.room_id().c_str(), _grp_msg_base));
        
        ACE_DEBUG((LM_DEBUG, "group_handler_base: set self id to %s\n",
                  ui->id().c_str()));
        // ui->login_id(_self.login_id());
        // ui->validation(_self.validation());
    }

    ACE_DEBUG((LM_DEBUG, "Node added done\n"));
    
    return 0;
}

int
group_handler_base::node_removed(const netcomgrp::node *n) {
    _house_adapter->node_removed(n);
        
    return 0;
}

// Start of netcomgrp::event_observer interface
int
group_handler_base::data_received(const void *data, 
                  size_t data_len, 
                  const netcomgrp::node *n) {
    ACE_DEBUG((LM_DEBUG, "group_handler_base: received data of size %d:\n%s\n",
      data_len, data));
    if (net_ip_block()->is_blocked(n->addr().get_ip_address())) {
        ACE_DEBUG((LM_DEBUG, "group_handler_base: data from blocked IP %s "
                  "received", n->addr().get_host_addr()));
        return 0;
    }
    _house_adapter->data_received(data, data_len, n);
    return 0;
}

int
group_handler_base::state_changed(int grp_state) {
    int mtype = -1;
    
    _house_adapter->state_changed(grp_state);
    
    switch (grp_state) {
    case netcomgrp::group::joining:
        mtype = GROUP_MESSAGE(message::group_joining); break;
    case netcomgrp::group::joined:
        net_report()->connected(_grp_msg_base);
        mtype = GROUP_MESSAGE(message::group_joined); break;
    case netcomgrp::group::leaving:
        net_report()->disconnected(_grp_msg_base);
        mtype = GROUP_MESSAGE(message::group_leaving); break;
    case netcomgrp::group::not_joined:
        net_report()->disconnected(_grp_msg_base);
        mtype = GROUP_MESSAGE(message::group_not_joined); break;
    }
    
    if (mtype > -1) gui_messenger()->send_msg(new message(mtype));
    
    if (_notify) _notify->group_state_changed(grp_state);
    
    return 0;
}

int
group_handler_base::error(int err, const char *reason) {
    if (_notify) _notify->group_error(err, reason);
    return 0;
}

int
group_handler_base::send(const std::string &msg, 
                         const std::string &channel,
                         unsigned seq) {
    _house_adapter->send(
        chat_gaming::pdu::header(chat_gaming::pdu::id_message, seq),
        chat_gaming::pdu::message(msg, channel)
    );
    return 0;
}

int
group_handler_base::send_room(const std::string &msg, 
                              const chat_gaming::room::id_type &rid,
                              unsigned seq)
{
    // Send to each participant of the group
    // the channel is the same as the room id...
    _house_adapter->send_to_room(
        chat_gaming::pdu::header(chat_gaming::pdu::id_message, seq),
        chat_gaming::pdu::message(msg, rid),
        rid
    );
    return 0;
}

void
group_handler_base::room_command(const chat_gaming::room::id_type &rid, 
                                 int command,
                                 unsigned seq)
{
    // Send to each participant of the group
    // the channel is the same as the room id...
    _house_adapter->send_to_room(
        chat_gaming::pdu::header(command, seq),
        rid,
        rid
    );
}

void
group_handler_base::refresh()
{
    ACE_DEBUG((LM_DEBUG, "group_handler_base::refresh\n"));
    
    // Iterate through each user and ask again info from it
    _house_type::user_iterator ui = _house.user_begin();
    for (; ui != _house.user_end(); ui++) {
        if (ui->id().node())
            _house_adapter->send_user_req(ui->id().node());
    }
}

void
group_handler_base::update(const chat_gaming::user &u) {
    ACE_DEBUG((LM_DEBUG, "group_handler_base::update user %s (room_id %s) grp %d\n",
              u.id().c_str(), u.room_id().c_str(), _grp_msg_base));
    const chat_gaming::user &s = _house_adapter->self_user();
    _house_adapter->self_user(u);
    if (_house_adapter->self_user().id().empty()) {
        _house_adapter->self_user().id(u.id());
    }
    _house_adapter->user_update(u);

    ACE_DEBUG((LM_DEBUG, "group_handler_base::update user self now %s (room_id %s) grp %d\n",
              s.id().c_str(), s.room_id().c_str(), _grp_msg_base));
}

void
group_handler_base::join(const chat_gaming::room &r, unsigned seq) {
    // Find room owner node
    chat_gaming::house::user_iterator ui = _house.user_find(r.owner_id());
    if (ui == _house.user_end()) {
        ACE_ERROR((LM_DEBUG, "group_handler_base::join: no owner user "
                  "found for room id %s, owner id %s\n",
                  r.id().c_str(), r.owner_id().c_str()));
        return;
    }
    
    _house_adapter->send_to(
        chat_gaming::pdu::header(chat_gaming::pdu::id_room_join, seq),
        chat_gaming::pdu::room_join(r),
        ui->id().node()
    );
}

void
group_handler_base::join_rsp(const chat_gaming::user::id_type &uid, int rsp, unsigned seq) {
    ACE_DEBUG((LM_DEBUG, "group_handler_base::join_rsp to %s, group %d\n",
              uid.c_str(), _grp_msg_base));
    
    chat_gaming::house::user_iterator ui = _house.user_find(uid.id_str());
    if (ui == _house.user_end()) {
        // Not really a bad error since the message can be sent via
        // several sources (DHT and centralized) and the user might be
        // associated with one source only. So this will return an error
        // when it is routed to the source that it didn't originate from.
        // But the other branch where it goes won't give error.
        ACE_ERROR((LM_WARNING, "group_handler_base::join_rsp: no user "
                  "found for id %s group %d\n", uid.c_str(), _grp_msg_base));
        return;
    }

    ACE_DEBUG((LM_DEBUG, "group_handler_base::join_rsp sending to node ptr %d\n",
              ui->id().node()));
    ACE_DEBUG((LM_DEBUG, "group_handler_base::join_rsp sending to id %s\n",
              ui->id().c_str()));

    _house_adapter->send_to(
        chat_gaming::pdu::header(rsp, seq),
        ui->id().node()
    );
}

void
group_handler_base::update(const chat_gaming::room &r) {
    _house_adapter->self_room(r);
    // Id must be updated separately
    _house_adapter->self_room().id(r.id());
    _house_adapter->room_update(r); 
}

// chat::comm::netcomgrp_adapter
void 
group_handler_base::user_inserted(chat_gaming::user &u, const netcomgrp::node *sender) {
    ACE_DEBUG((LM_DEBUG, "group_handler_base::user_added\n"));
    gui_messenger()->send_msg(new message_user(message::user, u, 0, _grp_msg_base));
    net_report()->new_user(u, _grp_msg_base);
}

void 
group_handler_base::user_erased(chat_gaming::user &u, const netcomgrp::node *sender) {
    ACE_DEBUG((LM_DEBUG, "group_handler_base::user_erased\n"));
    if (!u.room_id().empty()) {
        // Check that the user was hosting the room, no close otherwise
        _house_type::room_iterator ri = _house.room_find(u.room_id());
        if (ri != _house.room_end() &&
            ri->owner_id() == u.id()) 
        {       
            _room_closed_update(u.room_id(), u.id());
        }
    }
    gui_messenger()->send_msg(new message_user(message::user_left, u, 0, _grp_msg_base));
}

void 
group_handler_base::user_update(chat_gaming::user &old, chat_gaming::user &upd, const netcomgrp::node *sender) {
    ACE_DEBUG((LM_DEBUG, "group_handler_base::user_update id %s\n", upd.id().c_str()));
    ACE_DEBUG((LM_DEBUG, "group_handler_base::user_update old id %s\n", old.id().c_str()));

    // Since the id's node pointer is not transferred (as it is meaningless
    // at the other end), it is not yet set on the upd node, so set it to 
    // correct one obtained from the one already in.
    chat_gaming::user_key new_key(upd.id());
    new_key.node(old.id().node());
    upd.id(new_key);
    
    // Another special case is when user is first updated with it's info.
    // At first the user's information is empty and only a matching node ptr
    // is known about it. When user's information is updated, all normal
    // information is automatically copied. But the id() is not since it
    // is assumed to be immutable. But we want id_str to be updated there
    // the first time user is updated (ie. if it is still empty)
    if (old.id().id_str().empty()) {
        old.id(upd.id());
        ACE_DEBUG((LM_DEBUG, "group_handler_base::user_update updating "
                   "user id of the model (id_str is now %s)\n", 
                   old.id().id_str().c_str()));     
    }
    // Another special case is when the user crashes and restarts the
    // program before his absence is noticed. In that case the
    // user id's timestamp (embedded in id_str) will differ and 
    // the user display would get duplicates. To prevent that, check
    // if the old id differs from the new one and remove the old
    // then.
    else if (!old.id().id_str().empty() && 
             !upd.id().id_str().empty() &&
             old.id().id_str() != upd.id().id_str())
    {
        ACE_DEBUG((LM_DEBUG, "group_handler_base::user_update received "
            "id (%s) that differs from the old one (%s), removing the "
            "the old one\n", 
            upd.id().id_str().c_str(),
            old.id().id_str().c_str()
        ));
        // Simulate the deletion of the old user
        user_erased(old, sender);
        // But in here just update the model id
        old.id(upd.id());
    }
        
    // Check if the user's update will close some room
    if (!old.room_id().empty() && old.room_id() != upd.room_id()) {
        _house_type::room_iterator ri = _house.room_find(old.room_id());
        if (ri->owner_id() == upd.id())
            _room_closed_update(old.room_id(),upd.id());
    }
    gui_messenger()->send_msg(
        new message_user(message::user, upd, upd.sequence(), _grp_msg_base));

    net_report()->user_updated(old, upd, _grp_msg_base);        
}

void 
group_handler_base::_room_closed_update(
    const chat_gaming::room::id_type &id,
    const chat_gaming::user::id_type &sender_id) 
{
    ACE_DEBUG((LM_DEBUG, "group_handler_base::erasing "
              "room id %s\n", id.c_str()));
    _house_type::room_iterator i = _house.room_find(id);

    ACE_DEBUG((LM_DEBUG, "group_handler_base::room find done\n"));
    
    if (i != _house.room_end()) {
        ACE_DEBUG((LM_DEBUG, "group_handler_base:let us send a message\n"));
        gui_messenger()->send_msg(
          new message_room(message::room_remove, *i, sender_id, 0, _grp_msg_base)
        );
        ACE_DEBUG((LM_DEBUG, "group_handler_base:now the erasing\n"));
        _house.room_erase(i);
    } else {
        ACE_DEBUG((LM_DEBUG, "group_handler_base::room id %s not found",
                 id.c_str()));
    }
    ACE_DEBUG((LM_DEBUG, "group_handler_base:and off the function\n"));

}

void 
group_handler_base::user_data_available(chat_gaming::user &u, const netcomgrp::node *sender) {
    chat_gaming::pdu::header hdr;
    
    ACE_DEBUG((LM_DEBUG, "group_handler_base::user_data_available: " \
              "receiving user data\n"));
    _house_adapter->recv(&hdr);

    ACE_DEBUG((LM_DEBUG, "group_handler_base:go on\n"));
    
    message *smsg = NULL;
    
    switch (hdr.id()) {
    case chat_gaming::pdu::id_message:
    {
        chat_gaming::pdu::message data;
        _house_adapter->recv(&data);
        ACE_DEBUG((LM_DEBUG, "group_handler_base::user_data_available: " \
                  "received user data: %s\n", data.data().c_str()));
        
        // Hmm... what to do with the user data? Maybe send it to GUI
        // for processing or something
        smsg = new message_channel(
          message::send,
          data.data(),
          u.id(),
          data.channel(),
          hdr.sequence(),
          _grp_msg_base
        );
    }
        break;
    case chat_gaming::pdu::id_message_private:
    {
        chat_gaming::pdu::message data;
        _house_adapter->recv(&data);
        ACE_DEBUG((LM_DEBUG, "group_handler_base::user_data_available: " \
                  "received user data: %s\n", data.data().c_str()));
        ACE_DEBUG((LM_DEBUG, "group_handler_base::user_data_available: " \
                  "channel: %s\n", data.channel().c_str()));
        
        // Hmm... what to do with the user data? Maybe send it to GUI
        // for processing or something
        smsg = new message_channel(
          message::send_private,
          data.data(),
          u.id(),
          data.channel(),
          hdr.sequence(),
          _grp_msg_base
        );
    }
        break;
    case chat_gaming::pdu::id_private_refused:
    {
        chat_gaming::pdu::message data;
        _house_adapter->recv(&data);
        ACE_DEBUG((LM_DEBUG, "group_handler_base::user_data_available: " \
                  "received user data: %s\n", data.data().c_str()));
        ACE_DEBUG((LM_DEBUG, "group_handler_base::user_data_available: " \
                  "channel: %s\n", data.channel().c_str()));
        
        smsg = new message_channel(
          message::private_refused,
          data.data(),
          u.id(),
          data.channel(),
          hdr.sequence(),
          _grp_msg_base
        );
    }
        break;
    case chat_gaming::pdu::id_room_launch:
    {
        ACE_DEBUG((LM_DEBUG, "group_handler_base::id_launch\n"));
        chat_gaming::pdu::room_command data;
        _house_adapter->recv(&data);
        // _house_adapter->recv(&ruid);
        ACE_DEBUG((LM_DEBUG, "group_handler_base::user_data_available: " \
                  "received room launch command from user id %s " 
                  "to room id %s\n", 
                  u.id().c_str(),
                  data.room_id().c_str()));
        // ACE_DEBUG((LM_DEBUG, "group_handler_base::user_data_available: "
        //          "received launch command for room id: %s\n", ruid.c_str()));
        
        // Hmm... what to do with the user data? Maybe send it to GUI
        // for processing or something
        smsg = new message_room_command(
          message::room_launch,
          data.room_id(),
          data.target_user_id(),
          u.id(),
          hdr.sequence(),
          _grp_msg_base
        );
    }
        break;
    case chat_gaming::pdu::id_room_kick:
    {
        ACE_DEBUG((LM_DEBUG, "group_handler_base::id_room_kick\n"));
        chat_gaming::pdu::room_command data;
        _house_adapter->recv(&data);
        ACE_DEBUG((LM_DEBUG, "group_handler_base::user_data_available: " \
                  "received room kick command from user id %s " 
                  "about room id %s\n", 
                  u.id().c_str(),
                  data.room_id().c_str()));

        // TODO Disregard the kick message if it's not from the host of the
        // room
        smsg = new message_room_command(
          message::room_kick,
          data.room_id(),
          data.target_user_id(),
          u.id(),
          hdr.sequence(),
          _grp_msg_base
        );
    }
        break;
    case chat_gaming::pdu::id_room_join:
    {
        ACE_DEBUG((LM_DEBUG, "group_handler_base::id_room_join\n"));
        chat_gaming::pdu::room_join data;
        _house_adapter->recv(&data);
        ACE_DEBUG((LM_DEBUG, "group_handler_base::user_data_available: " \
                  "received room join request from user id %s " 
                  "to room id %s\n", 
                  u.id().c_str(),
                  data.room_id().c_str()));

        smsg = new message_room_join(
          message::room_join,
          data.room_id(), 
          data.password(),
          u.id(),
          hdr.sequence(),
          _grp_msg_base
        );  
    }
        break;
    case chat_gaming::pdu::id_room_join_ack:
        ACE_DEBUG((LM_DEBUG, "group_handler_base::user_data_available: " \
                  "received room join ack response\n"));
        smsg = new message_grouped(
            message::room_join_ack,
            u.id(),
            hdr.sequence(),
            _grp_msg_base
        );
        break;  
    case chat_gaming::pdu::id_room_join_full:
        ACE_DEBUG((LM_DEBUG, "group_handler_base::user_data_available: " \
                  "received room join full response\n"));
        smsg = new message_grouped(
            message::room_join_full,
            u.id(),
            hdr.sequence(),
            _grp_msg_base
        );
        break;  
    case chat_gaming::pdu::id_room_join_refused:
        ACE_DEBUG((LM_DEBUG, "group_handler_base::user_data_available: " \
                  "received room join nack response\n"));
        smsg = new message_grouped(
            message::room_join_nack,
            u.id(),
            hdr.sequence(),
            _grp_msg_base
        );
        break;  
    case chat_gaming::pdu::id_room_join_password:
        ACE_DEBUG((LM_DEBUG, "group_handler_base::user_data_available: " \
                  "received room join pass response\n"));
        smsg = new message_grouped(
            message::room_join_pass,
            u.id(),
            hdr.sequence(),
            _grp_msg_base
        );
        break;  
    case chat_gaming::pdu::id_user_blocked:
        ACE_DEBUG((LM_DEBUG, "group_handler_base::user_data_available: " \
                  "received user blocked\n"));
        smsg = new message_block_users(
            message::block_users,
            u,
            hdr.sequence(),
            _grp_msg_base
        );
        break;
    }
    
    if (smsg) gui_messenger()->send_msg(smsg);
}


void 
group_handler_base::room_update(
  chat_gaming::room &r, const netcomgrp::node *sender
) {
    ACE_DEBUG((LM_DEBUG, "group_handler_base::room_update, topic: %s\n",
               r.topic().c_str()));
    // Since the owner id node is not transferred over the network (being a pointer),
    // it must be set here manually. For that we need to assume the sender of
    // the update is the owner of the room (TODO a check here that at least
    // the id strings match)
    chat_gaming::user::id_type search_id(sender);
    _house_type::user_iterator ui = _house.user_find(search_id);    
    if (ui == _house.user_end()) {
        ACE_ERROR((LM_ERROR, "group_handler_base::room_update received from "
                  "netcomgrp node that was not found from model. Strange.\n"));
        return;
    }
    // id.node(sender);
    r.owner_id(ui->id());
    gui_messenger()->send_msg(
        new message_room(message::room, r, ui->id(), r.sequence(), _grp_msg_base)
    );
}
} // ns networking
