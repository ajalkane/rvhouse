#include <iostream>
#include <fstream>
#include <string>

#include "../../global.h"
#include "../../reporter/client.h"

#include "dht_operation.h"
#include "util.h"
#include "../../../file_util.h"
#include "../../worker.h"

namespace networking {
namespace kadc {
    
dht_operation::dht_operation(dht_target *t)
  : ::networking::dht_operation(t), _dht_client(NULL),
    _updating_contacts(false), _connect_after_update(false)
{
    std::string logfile = conf<std::string>("logfile", std::string());
    ACE_DEBUG((LM_DEBUG, "kadc::dht_operation: logfile '%s'\n", logfile.c_str()));
    if (!logfile.empty()) {
        logfile = app_rel_path(logfile);
        // First remove the old log file if it exists
        file_delete(logfile);
        ACE_DEBUG((LM_DEBUG, "kadc::dht_operation: logfile '%s'\n", logfile.c_str()));
        dht::kadc::client::logfile(logfile);
    }
}

std::string 
dht_operation::conf_kadc_ini() {
    return conf<std::string>("kadc_inifile", std::string());
    // return net_conf()->get_value("net_serverless_kadc/kadc_inifile");
}

std::string
dht_operation::conf_contacts_server() {
    return conf<std::string>("contacts_server", std::string());
    // return net_conf()->get_value("net_serverless_kadc/contacts_server");
}

std::string 
dht_operation::conf_contacts_dload() {
    return conf<std::string>("contacts_dload", std::string());
    // return net_conf()->get_value("net_serverless_kadc/contacts_dload");
}

dht_operation::~dht_operation() {
    
// Causes probs, disabled... kadC deinitialisation is too slow to wait
/*  if (_dht_client) {
        _dht_client->deinit();
        delete _dht_client;
        _dht_client = NULL;
    }
    */
    // But writing the contact files back is not too slow, so do that.
    // Only done if managed to get connected and the number of 
    // contacted nodes reasonably high.
    // Otherwise prefer to keep the old file
    if (_dht_client &&
        _dht_client->contacted_nodes() > 20) 
    {
        ACE_DEBUG((LM_DEBUG, "dht_operation::dtor writing ini file\n"));
        _dht_client->write_inifile();
    } else if (_dht_client) {
        ACE_DEBUG((LM_WARNING, "dht_operation: ini file not written, "
                   "contacted nodes: %d\n", _dht_client->contacted_nodes()));
    }
}

dht::client *
dht_operation::get_node() {
    return _dht_client;
}

int
dht_operation::prepare_connect() {
    if (get_node() && get_node()->in_state() == dht::client::disconnected) {
        // Make sure the node is deleted before doing a new connect
        ACE_DEBUG((LM_DEBUG, "kadc::dht_operation::prepare_connect: " \
                  "deleting kadc DHT node\n"));
        delete _dht_client;
        _dht_client = NULL;
    }
    if (!get_node()) {
        ACE_DEBUG((LM_DEBUG, "kadc::dht_operation: creating dht node\n"));
        std::string kfile = app_rel_path(conf_kadc_ini());
        // See if the kadc file exists
        if (!file_exists(kfile)) {
            _connect_after_update = true;
            ACE_DEBUG((LM_DEBUG, "%s kadc file not found, trying to retrieve " \
                      "a fresh copy\n", kfile.c_str()));
            update_contacts();
            return 0;
        }
        dht::name_value_map dht_conf;       
        dht_conf.set("init_file", kfile);
        _dht_client = new dht::kadc::client;
        _dht_client->init(dht_conf);
        _dht_client->find_threads  (conf<size_t>("find_threads",   0));
        _dht_client->find_duration (conf<size_t>("find_duration",  0));
        _dht_client->store_threads (conf<size_t>("store_threads",  0));
        _dht_client->store_duration(conf<size_t>("store_duration", 0));
        
        set_node(_dht_client);
        ACE_DEBUG((LM_DEBUG, "dht_operation calling dht_ready_to_connect\n"));
        _target->dht_ready_to_connect();
        ACE_DEBUG((LM_DEBUG, "dht_operation::prepare_connect returning\n"));
        return 0;
    }
    
    return ::networking::dht_operation::prepare_connect();
}

int
dht_operation::prepare_disconnect() {
    _connect_after_update = false;
    return ::networking::dht_operation::prepare_disconnect();
}
    
void
dht_operation::update_contacts() {
    ACE_DEBUG((LM_DEBUG, "kadc::dht_operation: " \
              "update contacts\n"));
    
    if (_updating_contacts) {
        ACE_DEBUG((LM_DEBUG, "kadc::dht_operation: " \
                             "contacts already being updated, nothing done\n"));
        return;
    }
    // Since udpate_contacts() needs to destroy the old _dht_client and
    // update contacts might be called from within _dht_client's callback,
    // schedule a wake up to the reactor to do so.
    _updating_contacts = true;
    net_report()->dht_bootstrap();
    ACE_Reactor::instance()->notify(this);
}

int
dht_operation::handle_exception(ACE_HANDLE) {
    ACE_DEBUG((LM_DEBUG, "kadc::dht_operation handle_exception called\n"));
    if (_updating_contacts) {
        ACE_DEBUG((LM_DEBUG, "kadc::dht_operation doing preparing to " \
                  "update contacts\n"));
        if (_dht_client && _dht_client->in_state() != dht::client::disconnected) {
            ACE_DEBUG((LM_ERROR, "kadc::dht_operation trying to " \
                      "update contacts but dht_client is in state %s\n",
                      _dht_client->in_state_str()));
            _updating_contacts = false;
        }
        ACE_DEBUG((LM_DEBUG, "kadc::dht_operation destroying old _dht_client\n"));
        delete _dht_client;
        _dht_client = NULL;
        
        gui_messenger()->send_msg(new message(message::dht_upd_nodes));
        
        std::string url = conf_contacts_server();
        std::string fil = app_rel_path(conf_contacts_dload());
        
        try {                                                
            _remote_file_saver.fetch(url, fil, this);
        } catch (const std::exception &e) {
            ACE_DEBUG((LM_DEBUG, "Updating server nodes failed with: '%s'. " \
                                "Used url '%s' and local file '%s'",
                                e.what(), url.c_str(), fil.c_str()));
            
            /*throw exceptionf(0, "Updating server nodes failed with: '%s'. " \
                                "Used url '%s' and local file '%s'",
                                e.what(), url.c_str(), fil.c_str());*/
            gui_messenger()->send_msg(new message(message::dht_upd_nodes_fail));
            _target->dht_update_server_nodes_done(1);

            // throw e;
        }
        
    }
    return 0;   
}

/*
 * remote_file_saver callback... called when updated server nodes list
 * has been fetched. 
 */
void
dht_operation::saver_done(int status, const char *extra) {
    _updating_contacts = false;
    
    if (status == remote_file_saver::status_error) {
        ACE_DEBUG((LM_ERROR, "server nodes update failed: %s\n", extra));
        gui_messenger()->send_msg(new message(message::dht_upd_nodes_fail));
        _target->dht_update_server_nodes_done(1);
    }
    if (status == remote_file_saver::status_ok) {
        ACE_DEBUG((LM_DEBUG, "nodes update file saved successfully\n"));
        std::ifstream overnet(app_rel_path(conf_contacts_dload()).c_str(),
                              std::ios::in | std::ios::binary);
        std::ofstream kadcini(app_rel_path(conf_kadc_ini()).c_str());
        util::kadc_pars kp(
            net_conf()->get<int>("net_serverless_kadc/port_udp",2304),
            net_conf()->get<int>("net_serverless_kadc/port_tcp",2301)
        );

        int contacts = util::overnet_contacts_to_kadc_ini(overnet, kadcini, kp);

        overnet.close();
        kadcini.close();
        
        if (contacts > 0) {
            gui_messenger()->send_msg(new message(message::dht_upd_nodes_done));
            _target->dht_update_server_nodes_done(0);
            if (_connect_after_update) {
                ACE_DEBUG((LM_DEBUG, "calling prepare_connect again\n"));
                prepare_connect();
                _connect_after_update = false;
            }
        } else {
            ACE_DEBUG((LM_DEBUG, "got zero contacts\n"));
            gui_messenger()->send_msg(new message(message::dht_upd_nodes_fail));
            _target->dht_update_server_nodes_done(1);
            file_delete(app_rel_path(conf_kadc_ini()));
        }
    }
}

int 
dht_operation::state_changed(int dht_state) {
    ACE_DEBUG((LM_DEBUG, "dht_operation:: " \
    "dht state change received (%s)\n", get_node()->state_str(dht_state)));

    if (dht_state == dht::client::disconnected) {
        ACE_DEBUG((LM_DEBUG, "kadc::dht_operation dht disconnect received, " \
                  "deleting node %d\n", _dht_client));
        // Can't delete node from here since this callback is called
        // from within _dht_client.
        // delete _dht_client;
        // ACE_DEBUG((LM_DEBUG, "kadc::dht_operation dtor called for dht_client\n"));
        // _dht_client = NULL;
    }
    return ::networking::dht_operation::state_changed(dht_state);
}

} // ns kadc
} // ns networking
