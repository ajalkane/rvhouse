#ifndef _CHAT_GAMING_USER_H_
#define _CHAT_GAMING_USER_H_

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>

#include <assert.h>
#include <string>
#include <time.h>

#include <netcomgrp/node.h>
#include <chat/user.h>

#include "user_key.h"
#include "room.h"

namespace chat_gaming {
    class user : public chat::user<user_key, room::id_type> {
    public:
        typedef chat::user<user_key> super;
        
        enum status_enum {
            status_chatting = 1,
            status_playing,
            status_away,
            status_dont_disturb,
        };
    private:
        const static int field_sharing_tracks = 0x1;
        const static int field_getting_tracks = 0x2;

        const static netcomgrp::addr_inet_type _addr_none;
        
        unsigned    _sequence;
        status_enum _status;
        std::string _status_msg;
        std::string _login_id;
        std::string _validation;
        int         _fields;
        // Need users current time so that relative times can be
        // calculated.
        time_t      _user_now;
        time_t      _user_status_time;
        
        // These not transferred, but inferred.
        struct _local_obj {
            std::string ip_as_string;
            std::string display_id;
            long user_us_time_delta;
            inline _local_obj() : user_us_time_delta(0) {}
        } _local;
        // std::string _status_str;
        
        friend class boost::serialization::access;
        
        // When the class Archive corresponds to an output archive, the
        // & operator is defined similar to <<.  Likewise, when the class Archive
        // is a type of input archive the & operator is defined similar to >>.
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            // If called for serialization to send, causes our time
            // be sent. If called for serialization from send, this
            // line doesn't matter but will be overwritten by the info
            // obtained from the packet.
            _user_now = time(NULL);
            
            ar & _sequence;
            ar & _id;
            ar & _room_id;
            ar & _login_id;
            ar & _validation;
            ar & _status;
            ar & _status_msg;
            ar & _fields;
            ar & _user_now;
            ar & _user_status_time;
            _init_extra_vars();
        }

#if 0
        // This crap doesn't compile for some reason
        template<class Archive>
        void save(Archive & ar, const unsigned int version)
        {
            // _user_now = time(NULL);
            serialize_common(ar, version);
        }       

        template<class Archive>
        void load(Archive & ar, const unsigned int version)
        {
            serialize_common(ar, version);
            _init_extra_vars();
        }       

        // BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif

        void _init_extra_vars();
        inline void _field_set(int field, bool v) {
            _fields = (v 
                       ? _fields | field
                       : _fields & ~field
                      );
        }
        
    public: 
        inline user(const netcomgrp::node *n = NULL);
        // inline user(const user &);

        // Compatibility accessors for chat::comm::netcomgrp_adapter
        inline const netcomgrp::node *node() const { return id().node(); }
        inline bool operator==(const netcomgrp::node &n) const {
            assert(id().node() != NULL);
            return *(id().node()) == n;
        }
        inline bool operator==(const user &o) const { return id() == o.id(); }
        
        inline unsigned sequence() const { return _sequence; }
        inline unsigned sequence(unsigned s) { return _sequence = s; }
        
        inline const std::string &login_id(const std::string &);
        inline const std::string &login_id() const;

        inline const std::string &validation(const std::string &);
        inline const std::string &validation() const;

        inline bool authenticated() const;
        
        inline const std::string &ip_as_string() const { return _local.ip_as_string; }
        inline const std::string &ip_as_string(const std::string &ip) { 
            return _local.ip_as_string = ip; 
        }
        inline const std::string &display_id()   const { return _local.display_id;   }
    
        inline status_enum status() const { return _status; }
        inline int status(status_enum s) {
            _user_status_time = time(NULL);
            return _status = s;
        }
        inline time_t status_time() const {
            time_t st = _local.user_us_time_delta + _user_status_time;
            return st;
        }
        
        inline const char *status_as_string() const { 
            return status_to_string(_status); 
        }
        static const char *status_to_string(int s);
        // Generates and sets user::id from login_id and validation status
        void generate_id(); 
        
        inline bool sharing_tracks() const { return _fields & field_sharing_tracks; }
        inline void sharing_tracks(bool v) { 
            _field_set(field_sharing_tracks, v);
        }
        inline bool getting_tracks() const { return _fields & field_getting_tracks; }
        inline void getting_tracks(bool v) { 
            _field_set(field_getting_tracks, v);
        }
        inline const netcomgrp::addr_inet_type &addr() const;       
    };

    user::user(const netcomgrp::node *n)
      : super(n), _sequence(0), _status(status_chatting), _fields(0),
        _user_now(time(NULL)),
        _user_status_time(time(NULL)) {
        _init_extra_vars();
      }

    inline const std::string &
    user::login_id(const std::string &s) { return _login_id = s; }
    inline const std::string &
    user::login_id() const { return _login_id; }

    inline const std::string &
    user::validation(const std::string &s) { return _validation = s; }
    inline const std::string &
    user::validation() const { return _validation; }
    
    // For now like this
    inline bool 
    user::authenticated() const { return _validation == "1"; }

    inline const netcomgrp::addr_inet_type &
    user::addr() const {
        if (!id().node()) return _addr_none;
        return id().node()->addr();
    }

} // ns chat_gaming

#endif //_CHAT_GAMING_USER_H_
