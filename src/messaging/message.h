#ifndef _MESSAGING_MESSAGE_H_
#define _MESSAGING_MESSAGE_H_

class message {
    int _id;
public:
    /// Message ids
    enum {
        connect = 1,
        disconnect,
        status_update,
        exit_now,
        login,
        login_err,
        login_done,
        login_fail,
        // Registering
        reg,
        reg_err,
        reg_done,
        reg_fail,

        dht_connecting,
        dht_connected,
        dht_disconnecting,
        dht_disconnected,
        dht_upd_nodes,
        dht_upd_nodes_done,
        dht_upd_nodes_fail,
        dht_peers_find_start,
        dht_peers_find_stop,
        dht_announce_start,
        dht_announce_stop,

        group_base,
        group_joining,
        group_joined,
        group_leaving,
        group_not_joined,

        // DHT (serverless) group messages
        dht_group_base,
        dht_group_joining,
        dht_group_joined,
        dht_group_leaving,
        dht_group_not_joined,

        // Centralized group messages
        ctz_group_base,
        ctz_group_joining,
        ctz_group_joined,
        ctz_group_leaving,
        ctz_group_not_joined,

        // Comes as message string holding the server name that could
        // not be reached.
        ctz_group_server_unreachable,

        // These come as message_string :
        // fetching - the site address
        // done     - the detected ip
        // fail     - the failure reason
        external_ip_fetching,
        external_ip_fetch_done,
        external_ip_fetch_fail,

        user,
        user_left,

        // user_room,
        // user_room_left,

        room,
        room_remove,
        room_launch,
        room_kick,
        room_join,
        room_join_pass,
        room_join_full,
        room_join_ack,
        room_join_nack,

        worker_exception,

        send,
        send_room,
        send_private,
        send_room_private,
        private_refused,

        refresh,

        version,
        version_force,
        version_fetch,

        rvtm_exited,

        block_users,
        global_ignore_fetch,
        global_ignore_list,

        last
    };

    message(int msg_id);
    virtual ~message();

    inline int id() const;
    inline int id(int i);

    virtual message *duplicate();
};

inline int message::id() const { return _id; }
inline int message::id(int i)  { return _id = i; }

#endif //_MESSAGING_MESSAGE_H_
