#include <netcomgrp/serverless/util.h>

#include "group_adapter_serverless.h"
// #include "group_handler_serverless.h"
#include "group_handler_base.h"
#include "kadc/dht_operation.h"

#include "../global.h"
#include "../worker.h"
#include "../reporter/client.h"

#include "../../file_util.h"
#include "../../regexp.h"
#include "../../util.h"
#include "../../messaging/message_string.h"
#include "../../messaging/message_user.h"

namespace networking {

group_adapter_serverless::group_adapter_serverless(
    ACE_Reactor *r
) : group_adapter_base(r), _dht_oper(NULL), _sgrp(NULL),
    _external_ip_fetching(false), 
    _self(NULL),
    _connect_try(0)
{
    // TODO needs global config.h or from qsettings
    _grp_id = net_conf()->get<std::string>(
        "net_serverless", "group_id", "house://1.0/re-volt"
    );
    ACE_DEBUG((LM_DEBUG, "group_adapter_serverless::ctor group_id %s\n",
              _grp_id.c_str()));
              
    // _dht_user_connecting = 0;

    _dht_oper = new kadc::dht_operation(this);
    _dht_oper->observer_attach(this); // , dht::event_observer::mask_state_changed);
}

group_adapter_serverless::~group_adapter_serverless() {
    delete _dht_oper;
}

netcomgrp::group  *
group_adapter_serverless::create_group() {
    _sgrp = new netcomgrp::serverless::group;
    
    _sgrp->observer_attach(
        this, netcomgrp::serverless::event_observer::mask_extra_state_changed
    );
    
    // Deannouncing is quite a slow operation so for now disable it when
    // leaving
    _sgrp->opts_disable(netcomgrp::serverless::group::opt_deannounce_with_leave);
    return _sgrp;
}

const std::string &
group_adapter_serverless::join_group() {
    return _grp_id;
}

group_handler_base *
group_adapter_serverless::create_handler(netcomgrp::group *g) {
    // return new group_handler_serverless(g, m);
    group_handler_base *h = new group_handler_base(g, message::dht_group_base);
    h->notify(this);
    return h;
}

void group_adapter_serverless::dht_ready_to_connect() {
    ACE_DEBUG((LM_DEBUG, "group_adapter_serverless::dht_ready_to_connect\n"));
    _connect_try++;
    // if (_dht_user_connecting)
    _dht_oper->get_node()->connect();
}

int group_adapter_serverless::connect(const chat_gaming::user &self) {
    if (_sgrp->in_state() != netcomgrp::group::not_joined) {
        ACE_DEBUG((LM_WARNING, "group_adapter_serverless: " \
                  "in state (%s) != not_joined, can't connect\n",
                  _sgrp->in_state_str()));
        return -1;
    }
    
    _connect_try = 0;
    _self        = self;
    _self.id(self.id());
    return _connect();
}

void
group_adapter_serverless::update(const chat_gaming::user &u) {
    if (u.status() == chat_gaming::user::status_playing) {
        if (_sgrp->active_mode() != netcomgrp::serverless::group::active_lazy) {
            ACE_DEBUG((LM_DEBUG, "group_adapter_serverless: setting lazy mode\n"));
            _sgrp->active_mode(netcomgrp::serverless::group::active_lazy);
        }
    } else {
        _sgrp->active_mode(netcomgrp::serverless::group::active_normal);    
    }
    
    group_adapter_base::update(u);
}

int group_adapter_serverless::_connect() {
    if (!_cond_group_join()) return 0;
    
    // variable_guard<int> g(_dht_user_connecting);
    // _dht_user_connecting++; //  = true;
    if (!_dht_oper->prepare_connect()) {
        // g.release();
        // Parallel to connecting the DHT, fetch external IP that is
        // used while announcing our presence.
        _external_ip_fetch();
        ACE_DEBUG((LM_DEBUG, "group_adapter_serverless: " \
                  "returned from _external_ip_fetch\n"));
    }
    
    return 0;
}

int
group_adapter_serverless::_cond_group_join() {
    int _dht_ok    = 0;
    int _ext_ip_ok = 0;
    
    if (_dht_oper->get_node() &&
        _dht_oper->get_node()->in_state() == dht::client::connected) 
    {
        _dht_ok = 1;
    }
    
    if (_external_ip != netcomgrp::addr_inet_type())
        _ext_ip_ok = 1;
        
    if (!_dht_ok && !_ext_ip_ok) return -1;
    if (_dht_ok ^ _ext_ip_ok) {
        ACE_DEBUG((LM_DEBUG, "group_adapter_serverless: " \
                  "can't join group yet as dht (%d) or external ip (%d) not "\
                  "ready yet\n", (int)_dht_ok, (int)_ext_ip_ok));
        return -1;
    }
    
    ACE_DEBUG((LM_DEBUG, "group_adapter_serverless: joining group\n"));
    _sgrp->dht_client(_dht_oper->get_node());
    _sgrp->addr_announce(_external_ip);
    return group_adapter_base::connect(_self);
}

void 
group_adapter_serverless::_external_ip_fetch() {
    if (_external_ip != netcomgrp::addr_inet_type()) {
        ACE_DEBUG((LM_DEBUG, "group_adapter_serverless: " \
                  "external IP known: %s:%d\n",
                  _external_ip.get_host_addr(), 
                  _external_ip.get_port_number()));
        return;
    }
    if (_external_ip_fetching) {
        ACE_DEBUG((LM_DEBUG, "group_adapter_serverless: " \
                  "external IP is being fetched\n"));
        return;
    }

    std::string ip      = net_conf()->get("net_serverless/ip");
    std::string ip_site = net_conf()->get("net_serverless/ip_site");
    
    if (!ip.empty()) {
        _external_ip_set(ip);
        return;
    }
    
    // Finally, try figuring out our IP from an external site
    // TODO there's something fishy in here... if the fetch throws
    // an exception due to not being able to resolve the address
    // of the ip site, the program crashes.
    bool error = false;
    try {
        ACE_DEBUG((LM_DEBUG, "group_adapter_serverless::fetching external ip\n"));
        // throw exceptionf(0, "Could not resolve address");
        
        // http::url testurl;
        // testurl.parse(ip_site);
        // _external_ip_fetcher.fetch(testurl, this);
        _external_ip_fetcher.fetch(ip_site, this);
    } catch (std::exception &e) {
        // fetch may throw an error if for example the site URL can
        // not be resolved.
        ACE_ERROR((LM_ERROR, "group_adapter_serverless::_external_ip_fetch exception '%s'\n",
                  e.what()));
        
        gui_messenger()->send_msg(
           new message_string(message::external_ip_fetch_fail, e.what()));
        error = true;
    }

    if (!error) {
        _external_ip_fetching = true;       
        gui_messenger()->send_msg(new message_string(message::external_ip_fetching,
                                            ip_site));
    }                                               
}

void
group_adapter_serverless::_external_ip_set(const std::string &ip) {
    ACE_DEBUG((LM_DEBUG, "group_adapter_serverless: " \
              "parsing IP: %s\n", ip.c_str()));
    u_short port = net_conf()->get<int>("net_serverless/port", 0);
    _external_ip.set(port, ip.c_str());

    ACE_DEBUG((LM_DEBUG, "group_adapter_serverless: " \
              "got parsed external IP: %s:%d\n",
              _external_ip.get_host_addr(), 
              _external_ip.get_port_number()));
              
    if (_external_ip.get_port_number() <= 0 ||
        _external_ip.get_ip_address() == INADDR_ANY ||
        _external_ip.get_ip_address() == INADDR_NONE)
    {
        throw exceptionf(0, 
                         "Could not parse IP address from %s:%s, " \
                         "got: %s:%d (ip:port)",
                         ip.c_str(), 
                         net_conf()->get("net_serverless/port").c_str(),
                          _external_ip.get_host_addr(), 
                          _external_ip.get_port_number());
    }
}

// http::handler interface
int 
group_adapter_serverless::handle_response(const http::response &resp) {
    
    std::vector<std::string> res(2);

    ACE_DEBUG((LM_DEBUG, "group_adapter_serverless: trying to find " \
              "IP address from response: \n%s\n", resp.content()));

    if (regexp::match("(\\d{1,3}(\\.\\d{1,3}){3})", resp.content(), res.begin())) {
        const std::string &ipstr = res[0];
        ACE_DEBUG((LM_DEBUG, "Got ip string: %s\n", ipstr.c_str()));
        _external_ip_set(ipstr);
        gui_messenger()->send_msg(new message_string(message::external_ip_fetch_done,
                                                ipstr));
        net_report()->ext_ip_detected(ipstr);
    } else {
        gui_messenger()->send_msg(new message_string(message::external_ip_fetch_fail,
                             "no suitable IP found from response"));            
        net_report()->ext_ip_failed();
    }
    
    _cond_group_join();
    return 0;
}

int 
group_adapter_serverless::handle_error(int reason, const char *details) {
    gui_messenger()->send_msg(new message_string(message::external_ip_fetch_fail,
                                            details));
    net_report()->ext_ip_failed();
    return 0;
}

int 
group_adapter_serverless::handle_close() {
    _external_ip_fetching = false;
    return 0;
}

void group_adapter_serverless::dht_ready_to_disconnect() {
    ACE_DEBUG((LM_DEBUG, "group_adapter_serverless: skipping disconnecting DHT for now\n"));
    // Do not do this at least for now... too slow process to wait
    // when quitting, and doing this way creates a race condition with
    // writing the inifile.
    // _dht_oper->get_node()->disconnect();    
}

int group_adapter_serverless::disconnect() {
    // _dht_user_connecting = 0;
    _connect_try = 0;
    if (_sgrp->in_state() != netcomgrp::group::not_joined) {
        ACE_DEBUG((LM_DEBUG, "group_adapter_serverless: leaving group\n"));
        return group_adapter_base::disconnect();
    }
    
    if (_dht_oper->prepare_disconnect())
        ACE_DEBUG((LM_WARNING, "group_adapter_serverless: prepare disconnect failed\n"));
        
    // dht will be disconnected after group has been left.
    return 0;
}

void
group_adapter_serverless::update_server_nodes() {
    _dht_oper->update_contacts();   
}

void
group_adapter_serverless::dht_update_server_nodes_done(int err) {
    // if (!err && _dht_user_connecting) {
    if (!err && _connect_try == 1) {
        _connect();
    }
}

int 
group_adapter_serverless::state_changed(int dht_state) {
    switch(dht_state) {
    case dht::client::connected:
        _cond_group_join();
        break;
    case dht::client::disconnected:
        // Make sure connect has not been tried multiple times already
        // if (_dht_user_connecting == 1) {
        if (_connect_try == 1) {
            ACE_DEBUG((LM_DEBUG, "DHT disconnect received without " \
                      "explicit disconnect request, assuming timeout\n"));
            ACE_DEBUG((LM_DEBUG, "Trying to update possible server nodes\n"));
            update_server_nodes();
        } else {
            // _dht_user_connecting = 0;
        }
        break;
    }
    
    return 0;
}

int 
group_adapter_serverless::search_result(
    const dht::key &k, 
    const dht::value &v)
{
    ACE_DEBUG((LM_DEBUG, "group_adapter_serverless::search_result: %s\n",
              k.c_str()));
    const dht::name_value_map &m = v.meta();

    if (!m.exists("netcomgrp")) return 0;
    
    netcomgrp::addr_inet_type addr;    
    using netcomgrp::serverless::util::addr_deserialize;
    if (addr_deserialize(&addr, v.data(), v.size()) <= 0) {
        ACE_ERROR((LM_WARNING, "group_adapter_serverless::search_result: "
                  "could not deserialize IP from data\n"));
        return 0;
    }
    
    if (_seen_ips.count(addr) == 0) {
        _seen_ips.insert(addr);
        ACE_DEBUG((LM_DEBUG, "group_adapter_serverless::search_result: "
                  "unseen peer %s:%d\n", 
                  addr.get_host_addr(),
                  addr.get_port_number()));
        net_report()->dht_ip_found(addr);                  
    }
    return 0;
}

int 
group_adapter_serverless::extra_state_changed(int estate) { 
    ACE_DEBUG((LM_DEBUG, "group_adapter_serverless::extra_state_changed " \
              "called with state %d\n", estate));
    
    message *msg = NULL;
    switch (estate) {
    case netcomgrp::serverless::group::peers_find_start:
        msg = new message(message::dht_peers_find_start); break;
    case netcomgrp::serverless::group::peers_find_stop:
        msg = new message(message::dht_peers_find_stop); break;
    case netcomgrp::serverless::group::announce_start:
        msg = new message(message::dht_announce_start); break;
    case netcomgrp::serverless::group::announce_stop:
        msg = new message(message::dht_announce_stop); break;
    }
    
    if (msg) gui_messenger()->send_msg(msg);
    return 0; 
}

void
group_adapter_serverless::group_state_changed(int state) {
    ACE_DEBUG((LM_DEBUG, "group_adapter_serverless::group_state_changed " \
              "called with state %d (%s)\n", state, _sgrp->state_str(state)));
    // ACE_DEBUG((LM_DEBUG, "group_adapter_serverless: dht_user_connecting: %d\n",
    //          _dht_user_connecting));
    switch (state) {
    case netcomgrp::group::not_joined:
        ACE_DEBUG((LM_DEBUG, "group_adapter_serverless: disconnecting\n"));
        // Now that group has been left, can disconnect the dht for real
        disconnect();
        break;  
    }
}



} // ns networking
