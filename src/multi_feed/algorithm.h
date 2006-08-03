#ifndef _MULTI_FEED_ALGORITHM_H_
#define _MULTI_FEED_ALGORITHM_H_

#include <string>
#include <sstream>
#include <functional>
#include <iterator>

#include <netcomgrp/common.h>

#include "../common.h"
#include "../chat_gaming/room.h"
#include "../chat_gaming/user.h"
#include "../model/house.h"
#include "../main.h"
#include "../app_options.h"
#include "../messaging/message.h"

#include "common.h"

namespace multi_feed {

// General templated function object that translates 
// a value to string. Templated parameters are the 
// object type that is translated, a function object that
// is used to access the value and the translator that
// is used to translate the object.
template <class ObjType, class ValueAccessor, class RetTrans>
class value_to_string_fobj {
    ValueAccessor _acc;
    RetTrans      _tr;
public:
    inline value_to_string_fobj(
      ValueAccessor accessor,
      RetTrans trans
    ) : _acc(accessor), _tr(trans) {}

    inline 
    std::string operator()(const ObjType &o, int grp) {
        return _tr(_acc(o), grp);
    }
};

// Dummy class if any... used to access just the object
template <class SelfType>
class self_accessor {
public:
    inline const SelfType &operator()(const SelfType &o) { return o; }
};

class dummy_string_to_string 
  : public std::binary_function<std::string, std::string, int> {
public:
    inline const std::string &operator()(const std::string &s, int) {
        return s;
    }
  };

class addr_to_string 
  : public std::binary_function<std::string, netcomgrp::addr_inet_type, int> {
public:
    std::string operator()(const netcomgrp::addr_inet_type &a, int) {
        std::ostringstream ostr;
        ostr << a.get_host_addr()
             << ":"
             << a.get_port_number();
        return ostr.str();
    }
};      

template <class FromType>
class streamable_to_string 
  : public std::binary_function<std::string, FromType, int> {
public:
    std::string operator()(const FromType &f, int) {
        std::ostringstream ostr;
        ostr << f;
        return ostr.str();
    }
};      

class user_to_status_string
  : public std::binary_function<std::string, chat_gaming::user, int> {
public:
    std::string operator()(const chat_gaming::user &u, int) {
        std::ostringstream ostr;
        ostr << u.status_as_string();
        if (u.status() != chat_gaming::user::status_chatting) {
            int mins = (time(NULL) - u.status_time()) / 60;
            ostr << " " << mins << " " << langstr("words/minutes_short");
        }
        return ostr.str();
    }
};      

class validation_to_string 
  : public std::binary_function<std::string, std::string, int> {
public:
    const char *operator()(const std::string &s, int) {
        ACE_DEBUG((LM_DEBUG, "validation_to_string: checking string %s\n",
                   s.c_str()));
        if (s == "1") return ""; // validated";
        return langstr("misc/user_not_validated");
    }
};      

#if 0
class is_natted_to_string
  : public std::binary_function<std::string, chat_gaming::user::id_type, int> {
public:
    const char *operator()(const chat_gaming::user::id_type &id, int grp) {
        if (grp == message::dht_group_base) {
            if (id.node() && id.node()->addr().get_port_number() !=
                NETCOMGRP_SERVERLESS_DEFAULT_PORT) 
            {
                return "User's connection seems to be behind NAT";
            }
        }
        return "";
    }
};      
#endif

class has_password_to_string
  : public std::binary_function<const char *, bool, int> {
public:
    const char *operator()(bool has_pass, int) {
        ACE_DEBUG((LM_DEBUG, "has_password_to_string::checking boolean %d\n",
                   has_pass));
        return (has_pass ? langstr("misc/pass_needed") : "");
    }
};      

class bool_to_yes_no_string 
  : public std::binary_function<std::string, bool, int> {
public:
    const char *operator()(bool v, int) {
        ACE_DEBUG((LM_DEBUG, "bool_to_string: %d\n", v));
        return v ? langstr("words/yes") : langstr("words/no");
    }
};      

class true_to_string 
  : public std::binary_function<const char *, bool, int> {
    const char *_str;
public:
    inline true_to_string(const char *str) : _str(str) {}
    const char *operator()(bool v, int) {
        ACE_DEBUG((LM_DEBUG, "true_to_string: %d\n", v));
        return v ? _str : "";
    }
};      

class room_id_to_string
  : public std::binary_function<std::string, chat_gaming::room::id_type, int> {
  // : public std::binary_function<std::string, std::string, int> {
public:
    std::string operator()(const chat_gaming::room::id_type &rid, int grp) {
    // std::string operator()(const std::string &rid, int) {
        
        ACE_DEBUG((LM_DEBUG, "room_id_to_string: room id %s group %d\n",
                   rid.c_str(), grp));
        model::house::house_type::room_iterator ri
          = house_model()->room_find(rid, grp);
        if (ri != house_model()->room_end()) {
            ACE_DEBUG((LM_DEBUG, "room_id_to_string: found topic '%s'\n",
                      ri->topic().c_str()));
            
            return ri->topic();
        } else if (!rid.empty()) {
            ACE_DEBUG((LM_DEBUG, "room_id_to_string: creating\n"));
            return langstr("words/creating");
        }
        
        return "";
    }
};

class user_id_to_string
  : public std::binary_function<std::string, chat_gaming::user::id_type, int> {
public:
    std::string operator()(const chat_gaming::user::id_type &uid, int grp) {        
        ACE_DEBUG((LM_DEBUG, "user_id_to_string: user id %s group %d\n",
                   uid.c_str(), grp));
        model::house::house_type::user_iterator ui
          = house_model()->user_find(uid, grp);
        if (ui != house_model()->user_end()) {
            ACE_DEBUG((LM_DEBUG, "user_id_to_string: found user '%s' (%s)\n",
                      ui->display_id().c_str(), ui->id().c_str()));
            return ui->display_id();
        }
        
        return "";
    }
    std::string operator()(const std::string &id) {
        ACE_DEBUG((LM_DEBUG, "user_id_to_string: user id %s from all groups\n",
                   id.c_str()));

        chat_gaming::user::id_type sid;
        sid.id_str(id);
                   
        model::house::house_type::user_iterator ui
          = house_model()->user_find(sid);
        if (ui != house_model()->user_end()) {
            ACE_DEBUG((LM_DEBUG, "user_id_to_string: found user '%s' (%s)\n",
                      ui->display_id().c_str(), ui->id().c_str()));
            return ui->display_id();
        }
        
        return "";
    }   
};

class user_id_to_host_string
  : public std::binary_function<std::string, chat_gaming::user::id_type, int> {
public:
    std::string operator()(const chat_gaming::user::id_type &uid, int grp) {        
        ACE_DEBUG((LM_DEBUG, "user_id_to_host_string: user id %s group %d\n",
                   uid.c_str(), grp));
        model::house::house_type::user_iterator ui
          = house_model()->user_find(uid, grp);
        if (ui != house_model()->user_end()) {
            std::ostringstream ostr(ui->display_id());
            if (ui->status() != chat_gaming::user::status_chatting) {
                ostr << " (" << user_to_status_string()(*ui, grp) << ")";
            }
            return ostr.str();
        }
        
        return "";
    }
};

class user_id_to_id_string
  : public std::binary_function<std::string, chat_gaming::user::id_type, int> {
public:
    std::string operator()(const chat_gaming::user::id_type &uid, int grp) {        
        ACE_DEBUG((LM_DEBUG, "user_id_to_id_string: user id %s group %d\n",
                   uid.c_str(), grp));
        model::house::house_type::user_iterator ui
          = house_model()->user_find(uid, grp);
        if (ui != house_model()->user_end()) {
            ACE_DEBUG((LM_DEBUG, "user_id_to_id_string: "
            "found user '%s' (%s) id string %s\n",
                      ui->display_id().c_str(), ui->id().c_str(),
                      ui->id().id_str().c_str()));
            return ui->id().id_str();
        }
        
        return "";
    }
};

class user_id_to_status_string
  : public std::binary_function<std::string, chat_gaming::user::id_type, int> {
public:
    std::string operator()(const chat_gaming::user::id_type &uid, int grp) {        
        ACE_DEBUG((LM_DEBUG, "user_id_to_status_string: user id %s group %d\n",
                   uid.c_str(), grp));
        model::house::house_type::user_iterator ui
          = house_model()->user_find(uid, grp);
        if (ui != house_model()->user_end()) {
            ACE_DEBUG((LM_DEBUG, "user_id_to_status_string: "
            "found user '%s' (%s) status %s\n",
                      ui->display_id().c_str(), ui->id().c_str(),
                      ui->status_as_string()));
            return ui->status_as_string();
        }
        
        return "";
    }
};

} // multi_feed

#endif /*_MULTI_FEED_ALGORITHM_H_*/
