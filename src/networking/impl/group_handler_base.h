#ifndef _NETWORKING_IMPL_GROUP_HANDLER_BASE_H_
#define _NETWORKING_IMPL_GROUP_HANDLER_BASE_H_

#include <netcomgrp/group.h>
#include <chat/house.h>
#include <chat/comm/netcomgrp_adapter.h>

#include "../group_handler.h"
#include "../../chat_gaming/house.h"
#include "../../chat_gaming/user.h"

namespace networking {
    class group_handler_notify {
    public:
        virtual void group_state_changed(int state);
        virtual void group_error(int err, const char *desc);
    };

    class group_handler_base : public netcomgrp::event_observer {
    protected:
        netcomgrp::group     *_grp;
        int                   _grp_msg_base;
        group_handler_notify *_notify;
        typedef chat_gaming::house   _house_type;
        typedef chat_gaming::user    _user_type;
        typedef chat::comm::netcomgrp_adapter<_house_type, group_handler_base>
          _house_adapter_type;
        _house_adapter_type  *_house_adapter;
        _house_type           _house;
        // _user_type            _self;

        void _room_closed_update(
            const chat_gaming::room::id_type &id,
            const chat_gaming::user::id_type &sender_id
        );
    public:
        group_handler_base(netcomgrp::group *grp, int msgbase);
        virtual ~group_handler_base();

        void handle_message(message *msg);
        void handle_room_command_message(message *msg);
        void handle_send_private_message(message *msg);
        void handle_send_notification_message(message *msg);
        void handle_private_refused     (message *msg);
        void handle_block_users         (message *msg);

#if 0
        void ip_blocked(uint32_t ip, unsigned seq);
#endif
        void remove_blocked_users(unsigned seq);

        inline void notify(group_handler_notify *n);
        inline void user_self(const chat_gaming::user &u);
        int send(const std::string &msg, const std::string &channel, unsigned seq);
        int send_room(const std::string &msg,
                      const chat_gaming::room::id_type &rid, unsigned seq);
        void room_command(const chat_gaming::room::id_type &rid,
                          int command, unsigned seq);
        void refresh();

        // Updates orginating from self
        virtual void update(const chat_gaming::room &r);
        virtual void update(const chat_gaming::user &u);

        virtual void join(const chat_gaming::room &r, unsigned seq);
        virtual void join_rsp(const chat_gaming::user::id_type &uid, int rsp_code, unsigned seq);

        // Start of netcomgrp::event_observer interface
        virtual int node_added  (const netcomgrp::node *);
        virtual int node_removed(const netcomgrp::node *);
        virtual int data_received(const void *,
                                  size_t,
                                  const netcomgrp::node *);

        virtual int state_changed(int);
        virtual int error(int err, const char *reason);
        // End of netcomgrp::event_observer interface

        // chat::comm::netcomgrp_adapter
        void user_inserted(chat_gaming::user &u, const netcomgrp::node *sender);
        void user_erased(chat_gaming::user &u, const netcomgrp::node *sender);
        void user_update(chat_gaming::user &u, chat_gaming::user &upd, const netcomgrp::node *sender);
        void user_data_available(chat_gaming::user &u, const netcomgrp::node *sender);

        void room_update(chat_gaming::room &r, const netcomgrp::node *sender);
    };

    inline void group_handler_base::notify(group_handler_notify *n) {
        _notify = n;
    }
    inline void group_handler_base::user_self(const chat_gaming::user &u) {
        ACE_DEBUG((LM_DEBUG, "group_handler_base: setting user %s\n",u.id().c_str()));
        ACE_DEBUG((LM_DEBUG, "group_handler_base: setting user was %s\n",
                  _house_adapter->self_user().id().c_str()));
        // _self = u;
        // _self.id(u.id());
        /*
        if (_house_adapter->self_user().login_id().empty()) {
            ACE_DEBUG((LM_DEBUG, "group_handler_base: setting user2\n"));
            _house_adapter->self_user(u);
            _house_adapter->self_user().id(u.id());
        }
        // ACE_DEBUG((LM_DEBUG, "group_handler_base: done! User id: %s\n",_self.id().c_str()));
        // ACE_DEBUG((LM_DEBUG, "TODO debug user id str: %s\n",
                  // u.id().id_str().c_str()));
        */
    }

} // ns networking

#endif //_NETWORKING_IMPL_GROUP_HANDLER_BASE_H_
