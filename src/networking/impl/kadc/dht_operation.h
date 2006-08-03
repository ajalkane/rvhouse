#ifndef _NETWORKING_IMPL_KADC_DHT_OPERATION_H_
#define _NETWORKING_IMPL_KADC_DHT_OPERATION_H_

#include <string>

#include <ace/Event_Handler.h> 
#include <dht/kadc/client.h>

#include "../../global.h"

#include "../../../config_file.h"
#include "../../remote_file_saver/fetcher.h"
#include "../dht_operation.h"

namespace networking {
namespace kadc {    
    class dht_operation 
      : public ::networking::dht_operation,
        public remote_file_saver::notify,
        public ACE_Event_Handler
    {
        remote_file_saver::fetcher    _remote_file_saver;
        dht::kadc::client              *_dht_client;

        bool _updating_contacts;
        bool _connect_after_update;
        
    protected:
        std::string conf_kadc_ini();
        std::string conf_contacts_server();
        std::string conf_contacts_dload();
        
    public:
        dht_operation(dht_target *t);
        virtual ~dht_operation();
        
        virtual dht::client *get_node();
        // Returns 0 if proceeding with connect so that 
        // dht_target::dht_ready_to_connect would be colled when connect
        // can be called.
        virtual int prepare_connect();
        // Returns 0 if proceeding with disconnect so that 
        // dht_target::dht_ready_to_disconnect would be colled when disconnect
        // can be called.
        virtual int prepare_disconnect();
        
        virtual void update_contacts();
                
        // dht::event_observer interface
        virtual int state_changed(int dht_state);
        // remote_file_saver::notify interface
        virtual void saver_done(int status, const char *extra);     
        
        // ACE_Event_Handler interface
        virtual int handle_exception(ACE_HANDLE);
        
        template <class T>
        inline const T conf(const char *key, const T &def) {
            return net_conf()->get("net_serverless_kadc",key,def);
        }
    };
} // ns kadc
} // ns networking

#endif //_NETWORKING_IMPL_KADC_DHT_OPERATION_H_
