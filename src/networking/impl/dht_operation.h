#ifndef _NETWORKING_IMPL_DHT_OPERATION_H_
#define _NETWORKING_IMPL_DHT_OPERATION_H_

/**
 * This file contains abstraction of the DHT operations done
 * by group_adapter_serverless. This is mainly to encapsulate differences
 * of updating contact lists with different DHT implementations
 */
 
#include <dht/client.h>

#include "../../config_file.h"
#include "../../messaging/messenger.h"

namespace networking {

    class dht_target {
    public:
        virtual ~dht_target();
        virtual void dht_ready_to_connect()    = 0;
        virtual void dht_ready_to_disconnect() = 0; 
        virtual void dht_update_server_nodes_done(int err) = 0;
    };
    
    class dht_operation : public dht::event_observer {
    protected:
        dht_target  *_target;

        dht::event_observer *_obs;
        
        void set_node(dht::client *n);
    public:
        dht_operation(dht_target *t);
        virtual ~dht_operation();
        
        virtual dht::client *get_node() = 0;
        // Returns 0 if proceeding with connect so that 
        // dht_target::dht_ready_to_connect would be colled when connect
        // can be called.
        virtual int prepare_connect();
        // Returns 0 if proceeding with disconnect so that 
        // dht_target::dht_ready_to_disconnect would be colled when disconnect
        // can be called.
        virtual int prepare_disconnect();
        
        virtual void update_contacts();
        
        // like dht::client::observer_attach ...
        virtual void observer_attach(dht::event_observer *obs);
        
        // dht::event_observer interface
        virtual int state_changed(int dht_state);
        virtual int search_result(const dht::key &k, const dht::value &v);
    };

} // ns networking

#endif //_NETWORKING_IMPL_DHT_OPERATION_H_
