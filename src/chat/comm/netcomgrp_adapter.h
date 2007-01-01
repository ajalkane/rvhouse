#ifndef _CHAT_COMM_NETCOMGRP_ADAPTER_H_
#define _CHAT_COMM_NETCOMGRP_ADAPTER_H_

#include <iostream>
#include <sstream>
#include <utility>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <netcomgrp/event_observer.h>
#include <netcomgrp/group.h>


#include "pdu/header.h"

// #include "house_serializer.h"

namespace chat {
namespace comm {

template <typename HouseType>
class netcomgrp_adapter_notify {
public:
    typedef HouseType house_type;
    typedef typename HouseType::user_type user_type;
    typedef typename HouseType::room_type room_type;
    
    inline void user_inserted(user_type &u, const netcomgrp::node *sender) {}
    inline void user_erased  (user_type &u, const netcomgrp::node *sender) {}
    inline void user_update  (user_type &old, user_type &upd, const netcomgrp::node *sender) {}

    inline void room_update  (room_type &u, const netcomgrp::node *sender) {}
        
    // When this is called, netcomgrp_adapter::recv can be used
    // to read the user data
    inline void user_data_available(user_type &u) {}
};

template <typename HouseType, 
          typename NotifyType = netcomgrp_adapter_notify<HouseType> >
class netcomgrp_adapter : public netcomgrp::event_observer {
public:
    enum {
        id_user_req = 1,
        id_user_update,
        id_user_data,
        
        id_room_update,
    };
    typedef HouseType house_type;
    typedef typename HouseType::user_type user_type;
    typedef typename HouseType::room_type room_type;

    typedef boost::archive::text_oarchive oarchive_type;
    typedef boost::archive::text_iarchive iarchive_type;
    
    netcomgrp_adapter(netcomgrp::group *grp, 
                      HouseType *model, 
                      NotifyType *n)
        : _grp(grp), _house(model), _notify(n), _tmpiarchive(NULL) {}
private:
    netcomgrp::group *_grp;
    HouseType        *_house;
    NotifyType       *_notify;
    // house_serializer<HouseType> _ser;
    std::string _data;
    user_type _self_user;
    room_type _self_room;
    
    // Only used to receive recursively user data from within a callback,
    // otherwise is set as NULL
    iarchive_type *_tmpiarchive;

    template <typename VarType>
    class var_guard {
        VarType &_var;
        VarType  _val;
    public:
        var_guard(VarType v) : _var(v), _val(v) {}
        ~var_guard() { _var = _val; }
    };
public: 
    inline user_type &self_user(const user_type &u) { return _self_user = u; }
    inline room_type &self_room(const room_type &r) { return _self_room = r; }

    inline user_type &self_user() { return _self_user; }
    inline room_type &self_room() { return _self_room; }

    // Method to send to all participants a data type 
    // (must be serializable type)
    template <typename DataType>
    int send(const DataType &data) {
        std::ostringstream oss;
        oarchive_type oa(oss);
        const pdu::header hdr(id_user_data);
        oa << hdr;
        oa << data;
        
        ACE_DEBUG((LM_DEBUG, "netcomgrp_adapter::sending data of " \
                   "size %d:\n%s\n", 
                  oss.str().size(),
                  oss.str().c_str()));
            
        _grp->send(oss.str().c_str(), oss.str().size());
        return 0;
    }

    // TODO Stupid hack for sending more than one consequent data type at once.
    // When time permits replace with more general approach
    template <typename DataType1, typename DataType2>
    int send(const DataType1 &data1, const DataType2 &data2) {
        std::ostringstream oss;
        oarchive_type oa(oss);
        const pdu::header hdr(id_user_data);
        oa << hdr;
        oa << data1;
        oa << data2;
        
        ACE_DEBUG((LM_DEBUG, "netcomgrp_adapter::sending data of " \
                   "size %d:\n%s\n", 
                  oss.str().size(),
                  oss.str().c_str()));
            
        _grp->send(oss.str().c_str(), oss.str().size());
        return 0;
    }

    template <typename DataType1, typename DataType2>
    int send_to(const DataType1 &data1, const DataType2 &data2, 
                const netcomgrp::node *n) 
    {
        std::ostringstream oss;
        oarchive_type oa(oss);
        const pdu::header hdr(id_user_data);
        oa << hdr;
        oa << data1;
        oa << data2;
        
        ACE_DEBUG((LM_DEBUG, "netcomgrp_adapter::sending data of " \
                   "size %d:\n%s\n", 
                  oss.str().size(),
                  oss.str().c_str()));
            
        _grp->send(oss.str().c_str(), oss.str().size(), n);
        return 0;
    }


    template <typename DataType1>
    int send_to(const DataType1 &data1, 
                const netcomgrp::node *n) 
    {
        std::ostringstream oss;
        oarchive_type oa(oss);
        const pdu::header hdr(id_user_data);
        oa << hdr;
        oa << data1;
        
        ACE_DEBUG((LM_DEBUG, "netcomgrp_adapter::sending data of " \
                   "size %d:\n%s\n", 
                  oss.str().size(),
                  oss.str().c_str()));
            
        _grp->send(oss.str().c_str(), oss.str().size(), n);
        return 0;
    }

    template <typename DataType1>
    int send_to_room(const DataType1 &data1, 
                     const typename room_type::id_type &rid) 
    {
        std::ostringstream oss;
        oarchive_type oa(oss);
        const pdu::header hdr(id_user_data);
        oa << hdr;
        oa << data1;
        
        // Collect all participants in the room (TODO interface for this)
        typedef std::list<typename user_type::id_type> list_type;
        list_type user_ids;
        std::insert_iterator<list_type> usr_ids(user_ids, user_ids.begin());
        _house->user_ids_in_room(rid, usr_ids);
        
        ACE_DEBUG((LM_DEBUG, "netcomgrp_adapter::sending data of " \
                   "size %d to room of size %d:\n%s\n",
                  oss.str().size(), user_ids.size(),
                  oss.str().c_str()));
        
        typename list_type::iterator i = user_ids.begin();
        for (; i != user_ids.end(); i++) {
            // TODO the interface for netcomgrp to send to several nodes
            // at once!
            // TODO this one assumes user id() has node method... may be an
            // unfortunate assumption, but will do for now.
            _grp->send(oss.str().c_str(), oss.str().size(), i->node());
        }
        return 0;
    }

    template <typename DataType1, typename DataType2>
    int send_to_room(const DataType1 &data1, const DataType2 &data2, 
                     const typename room_type::id_type &rid) 
    {
        std::ostringstream oss;
        oarchive_type oa(oss);
        const pdu::header hdr(id_user_data);
        oa << hdr;
        oa << data1;
        oa << data2;
        
        // Collect all participants in the room (TODO interface for this)
        typedef std::list<typename user_type::id_type> list_type;
        list_type user_ids;
        std::insert_iterator<list_type> usr_ids(user_ids, user_ids.begin());
        _house->user_ids_in_room(rid, usr_ids);
        
        ACE_DEBUG((LM_DEBUG, "netcomgrp_adapter::sending data of " \
                   "size %d to room of size %d:\n%s\n",
                  oss.str().size(), user_ids.size(),
                  oss.str().c_str()));
        
        typename list_type::iterator i = user_ids.begin();
        for (; i != user_ids.end(); i++) {
            // TODO the interface for netcomgrp to send to several nodes
            // at once!
            // TODO this one assumes user id() has node method... may be an
            // unfortunate assumption, but will do for now.
            _grp->send(oss.str().c_str(), oss.str().size(), i->node());
        }
        return 0;
    }

    inline void send_user_req(const netcomgrp::node *to) {
        _send_user_req(to);
    }
    
    // Should only be called from within NotifyType::user_data_available()
    template <typename DataType>
    int recv(DataType *data) {
        if (_tmpiarchive == NULL) {
            ACE_DEBUG((LM_DEBUG, "netcomgrp_adapter::recv called where shouldnt\n"));
            assert(0);
        }
        
        *_tmpiarchive >> *data;
        return 0;
    }
    
    // Updates the room info, creating a new room if necessary or
    // destroying the old if no players remain.
    const room_type &room_update(const room_type &r) {
        ACE_DEBUG((LM_DEBUG, "chat::netcomgrp_adapter: room_update: %s\n", 
                   r.id().c_str()));
        
        // Please, do send to everyone concerned the room update information.
        // Make a user info response
        _send_room_update(r);       
        return r;
    }

    // Updates the user info, creating a new user if necessary or
    // destroying the old if no players remain.
    const user_type &user_update(const user_type &u) {
        ACE_DEBUG((LM_DEBUG, "chat::netcomgrp_adapter: user_update: %s\n", 
                   u.id().c_str()));
        
        // Please, do send to everyone concerned the room update information.
        // Make a user info response
        _send_user_update(u);
        return u;
    }
    
    // Start of netcomgrp::event_observer interface
    virtual int node_added  (const netcomgrp::node *n) 
    {
        typename house_type::user_iterator ui = _house->user_insert(user_type(n));
        if (ui == _house->user_end()) {
            ACE_DEBUG((LM_DEBUG, "chat::netcomgrp_adapter: could not add " \
                      "new netcomgrp node\n"));
        } else {
            _notify->user_inserted(*ui, n);
            _send_user_req(n);
        }
        return 0;
    }
    
    virtual int node_removed(const netcomgrp::node *n)
    {
        ACE_DEBUG((LM_DEBUG, "chat::netcomgrp_adapter: node: %d\n", n));
        typename house_type::user_iterator ui = _house->user_find(user_type(n));

        if (ui != _house->user_end()) {
            _notify->user_erased(*ui, n);
            if (!_house->user_erase(ui)) {
                ACE_ERROR((LM_ERROR, "chat::netcomgrp_adapter: " \
                "ERROR: SHOULD NOT HAPPEN... user_found but erase failed\n"));
            }
            
        } else {
            ACE_DEBUG((LM_DEBUG, "chat::netcomgrp_adapter::node_removed " \
            "no matching user found for %s:%d",
            n->addr().get_host_addr(), n->addr().get_port_number()));
        }               
        return 0;
    }
    
    virtual int data_received(const void *data, 
                              size_t len, 
                              const netcomgrp::node *n)
    {
        ACE_DEBUG((LM_DEBUG, "chat::netcomgrp_adapter: " \
                  "data received %d bytes ptr %d\n", len, data));
        ACE_DEBUG((LM_DEBUG, "chat::netcomgrp_adapter: " \
                  "from %s:%d\n", 
                  n->addr().get_host_addr(),
                  n->addr().get_port_number()));

        try {
            // TODO this does unnecessary copying of data, so should fix
            // it to use the received data directly as a stream. But since
            // the data is typically very small, this will do for now
            std::istringstream iss(
              std::string(static_cast<const char *>(data), len));
            iarchive_type ia(iss);
            pdu::header hdr(-1);
            ia >> hdr;
            
            switch (hdr.payload_type()) {
            case id_user_req:
                _recv_user_req(ia, hdr, n);
                break;
            case id_user_data:
                _recv_user_data(ia, hdr, n);
                break;
            case id_room_update:
                _recv_room_update(ia, hdr, n);
                break;
            case id_user_update:
                _recv_user_update(ia, hdr, n);
                break;
            default:
                ACE_ERROR((LM_ERROR, "Unrecognized header type %d received\n",
                          hdr.payload_type()));
            }
        } catch (const std::exception &e) {
            _tmpiarchive = NULL;
            ACE_DEBUG((LM_ERROR, "chat::netcomgrp_adapter::recv: " \
                      "exception: %s\n", e.what()));
        } catch (...) {
            _tmpiarchive = NULL;
            ACE_DEBUG((LM_ERROR, "chat::netcomgrp_adapter::recv: " \
                      "unrecognized exception\n"));
        }
        
        return 0;
    }
        
    /*
    virtual int state_changed(int);  */

private:
    void _recv_user_req(iarchive_type &ia, const pdu::header &hdr, 
                        const netcomgrp::node *from) 
    {
        ACE_DEBUG((LM_DEBUG, "user_req received\n"));
        ACE_DEBUG((LM_DEBUG, "self_user id/room_id: %s/%s, hosting room id: %s\n",
                  _self_user.id().c_str(),
                  _self_user.room_id().c_str(),
                  _self_room.id().c_str()));

        // Since it is theoretically possible that the from node is
        // deleted when send is called (due to timeout etc.), do
        // a duplicate so that _send_room_update would not cause 
        // any problems.
        netcomgrp::node *send_to = from->duplicate();
        std::auto_ptr<netcomgrp::node> node_guard(send_to);
        
        _send_user_update(_self_user, send_to);
        // If hosting room, send an update.
        if (_self_user.room_id() != typename room_type::id_type() &&
            _self_user.room_id() == _self_room.id())
        {
            ACE_DEBUG((LM_DEBUG, "_recv_user_req::sending room update\n"));
            _send_room_update(_self_room, send_to);
        }
    }

    void _recv_user_data(iarchive_type &ia, const pdu::header &hdr, 
                         const netcomgrp::node *from)
    {
        typename house_type::user_iterator ui = 
          _house->user_find(typename user_type::id_type(from));

        if (ui != _house->user_end()) {
            // var_guard prepares us to restore _tmpiarchive to NULL even
            // in the case of exception.
            var_guard<iarchive_type *> g(_tmpiarchive);
            _tmpiarchive = &ia;         
            _notify->user_data_available(*ui, from);
        }
    }
    void _recv_room_update(iarchive_type &ia, const pdu::header &hdr, 
                           const netcomgrp::node *from)
    {
        room_type room_info;
        ia >> room_info;
        ACE_DEBUG((LM_DEBUG, "chat::netcomgrp_adapter::recv_room_update for " \
          "room id '%s', owner id '%s'\n", 
          room_info.id().c_str(),
          room_info.owner_id().c_str()));

        // Ignore room updates with empty id (WaVe bug)
        // typename room_type::id_type empty_id 
        if (room_info.id() == typename room_type::id_type()) {
            ACE_DEBUG((LM_WARNING, "chat::netcomgrp_adapter::recv_room_update " \
              "with empty room id received, ignoring"));
            return;
        }
        typename house_type::room_iterator ri = _house->room_find(room_info);

        if (ri != _house->room_end()) {
            ACE_DEBUG((LM_DEBUG, "chat::netcomgrp_adapter: updating existing room info\n"));

            typename house_type::user_iterator ui = 
              _house->user_find(ri->owner_id());              

            if (ui == _house->user_end()) {
                ACE_ERROR((LM_ERROR, "chat::netromcgrp_adapter: "
                "no user found for id %s, tried to update room %s\n",
                ri->owner_id().c_str(), ri->id().c_str()));
                return;
            }
            // Only allow owner of the room send updates to it
            if (*ui == *from) {
                *ri = room_info;
            } else {
                ACE_ERROR((LM_ERROR, "chat::netcomgrp_adapter: "
                "sender of room info was not owner of room, update not allowed. "
                "Sender/Owner: %s:%d/%s:%d\n",
                from->addr().get_host_addr(),
                from->addr().get_port_number(),
                ui->node()->addr().get_host_addr(),
                ui->node()->addr().get_port_number()));
                return;
            }
                
        } else {
            ACE_DEBUG((LM_DEBUG, "chat::netcomgrp_adapter: inserting new room info\n"));
            ri = _house->room_insert(room_info);
        }

        if (ri != _house->room_end()) {
            _notify->room_update(*ri, from);
        } else {
            ACE_ERROR((LM_DEBUG, "chat::netcomgrp_adapter: room update failed " 
                      "for room id '%s'!\n", room_info.id().c_str()));
        }
    }

    void _recv_user_update(iarchive_type &ia, const pdu::header &hdr, 
                           const netcomgrp::node *from)
    {
        ACE_DEBUG((LM_DEBUG, "user_update\n"));
        
        typename house_type::user_iterator ui = 
          _house->user_find(user_type(from));

        if (ui != _house->user_end()) {
            user_type user_info(from);
            ia >> user_info;

            // Ignore user updates with empty id
            if (user_info.id() == typename user_type::id_type()) {
                ACE_DEBUG((LM_WARNING, "chat::netcomgrp_adapter::recv_user_update " \
                  "with empty user id received, ignoring"));
                return;
            }

            _notify->user_update(*ui, user_info, from);

            *ui = user_info;            
        } else {
            ACE_DEBUG((LM_DEBUG, "chat::netcomgrp_adapter::recv_user_update " \
              "no matching user found for %s:%d",
              from->addr().get_host_addr(), 
              from->addr().get_port_number()));
        }
    }

    template <typename T>
    void _send_type(int hdr_id, const T &snd_type, const netcomgrp::node *to = NULL)
    {
        // Make a user info response
        std::ostringstream oss;
        oarchive_type oa(oss);
        const pdu::header hdr(hdr_id);
        oa << hdr;
        oa << snd_type;
        
        if (to != NULL) {
            ACE_DEBUG((LM_DEBUG, "netcomgrp_adapter::sending resp of " \
                       "size %d to %s:%d:\n%s\n", 
                       oss.str().size(),
                       to->addr().get_host_addr(),
                       to->addr().get_port_number(),
                       oss.str().c_str()));
            
            _grp->send(oss.str().c_str(), oss.str().size(), to);
        } else {
            ACE_DEBUG((LM_DEBUG, "netcomgrp_adapter::sending resp of " \
                       "size %d to all participants:\n%s\n", 
                       oss.str().size(),
                       oss.str().c_str()));
            _grp->send(oss.str().c_str(), oss.str().size());
        }
    }

    void _send_user_req(const netcomgrp::node *to) {
        ACE_DEBUG((LM_DEBUG, "netcomgrp_adapter::_send_user_req to " \
                   "%s:%d\n", 
                   to->addr().get_host_addr(),
                   to->addr().get_port_number()
                   ));      
        // Make a user info request
        std::ostringstream oss;
        oarchive_type oa(oss);
        const pdu::header hdr(id_user_req);
        oa << hdr;
        ACE_DEBUG((LM_DEBUG, "netcomgrp_adapter:: sending req of " \
                   "size %d:\n%s\n", 
                  oss.str().size(),
                  oss.str().c_str()));
            
        _grp->send(oss.str().c_str(), oss.str().size(), to);
        ACE_DEBUG((LM_DEBUG, "netcomgrp_adapter::_send_user_req returning\n"));
    }
    void _send_room_update(const room_type &r, const netcomgrp::node *to = NULL) {
        ACE_DEBUG((LM_DEBUG, "netcomgrp_adapter::_send_room_update\n"));
        _send_type(id_room_update, r, to);
    }

    void _send_user_update(const user_type &u, const netcomgrp::node *to = NULL) {
        ACE_DEBUG((LM_DEBUG, "netcomgrp_adapter::_send_user_update\n"));
        
        _send_type(id_user_update, u, to);
    }

};

} // ns comm
} // ns chat

#endif //_ROOM_NETCOMGRP_ADAPTER_H_
