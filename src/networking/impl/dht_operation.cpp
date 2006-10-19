#include "../global.h"
#include "../reporter/client.h"
#include "dht_operation.h"

namespace networking {

dht_target::~dht_target() {}

dht_operation::dht_operation(dht_target *t)
  : _target(t), _obs(NULL) {}
  
dht_operation::~dht_operation() {}

void
dht_operation::set_node(dht::client *n) {
    // Install observer for reporting DHT state changes with messenger
    // (to gui)
    n->observer_attach(this, dht::event_observer::mask_state_changed);
    n->observer_attach(this, dht::event_observer::mask_search_result);
}

int
dht_operation::prepare_connect() {
    dht::client *n = get_node();
    if (n) {
        if (n->in_state() == dht::client::connected) {
            ACE_DEBUG((LM_DEBUG, "dht_operation: " \
            "dht node is connected already"));
            return -1;
        }
        
        if (n->in_state() != dht::client::disconnected) {
            ACE_DEBUG((LM_WARNING, "dht_operation: " \
            "dht node is in state (%s) != disconnected, can't connect\n",
            n->in_state_str()));
            return -1;
        }
        
        _target->dht_ready_to_connect();
        return 0;
    }
    
    return -1;
}

int
dht_operation::prepare_disconnect() {
    dht::client *n = get_node();
    if (n) {
        _target->dht_ready_to_disconnect();     
        return 0;
    }
    
    return -1;
}
    
void
dht_operation::update_contacts() {}
    
void
dht_operation::observer_attach(dht::event_observer *obs /*, int events_mask */) {
    // Dont't care about events_mask, what target class
    // defines will get called
    _obs = obs;
}

int 
dht_operation::state_changed(int dht_state) {
    int mtype = 0;
    
    switch (dht_state) {
    case dht::client::connecting:    mtype = message::dht_connecting;    break;
    case dht::client::connected:     
        mtype = message::dht_connected;     
        net_report()->dht_connected();        
        break;
    case dht::client::disconnecting: mtype = message::dht_disconnecting; break;
    case dht::client::disconnected:  
        mtype = message::dht_disconnected;
        net_report()->dht_connected();        
        break;
    default:
        // get_node() might return NULL so don't do this here
        // ACE_DEBUG((LM_WARNING, "dht_operation: " 
        // "warning: unrecognized dht state change received (%d:%s)\n", 
        // dht_state, get_node()->state_str(dht_state)));
        break;
    }
    
    if (mtype) gui_messenger()->send_msg(new message(mtype));
    
    // Relay the state change
    if (_obs) _obs->state_changed(dht_state);
    
    return 0;
}

int
dht_operation::search_result(const dht::key &k, const dht::value &v) {
    return (_obs ? _obs->search_result(k, v) : 0);   
}


} // ns networking
