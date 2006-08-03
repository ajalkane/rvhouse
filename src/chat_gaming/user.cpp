#include <sstream>
#include <functional>
#include <iterator>
#include <stdlib.h>

#include "user.h"

namespace chat_gaming {

const netcomgrp::addr_inet_type user::_addr_none = netcomgrp::addr_inet_type();

class ip_mask 
  : public std::unary_function<std::string::value_type,std::string::value_type>
{
    int _dot_count;
public:
    inline ip_mask() : _dot_count(0) {}
    inline result_type operator()(argument_type a) {
        if (a == '.') {
            _dot_count++;
            return a;
        }
        // 202.??.??.80
        return (_dot_count >= 1 && _dot_count < 3 ? '?' : a);
    }
};

void 
user::generate_id() {
    std::ostringstream user_id;
    user_id << time(NULL) << ":"
            << login_id() << ":"
            << (authenticated() ? "1" : "0");

    user_key new_id(id());
    new_id.id_str(user_id.str());
    id(new_id);
    _init_extra_vars(); 
}

void
user::_init_extra_vars() {
    _local.user_us_time_delta = (long)(time(NULL) - _user_now);

    if (id().node()) {
        // Generate IP string from the node address
        const netcomgrp::addr_inet_type &a = id().node()->addr();
        _local.ip_as_string = a.get_host_addr();
    }
    
    _local.display_id = login_id();
    ACE_DEBUG((LM_DEBUG, "chat_gaming::user::_init_extra_vars: "
              "display_id/login_id/ip/time_delta: %s/%s/%s/%d\n", 
              _local.display_id.c_str(),
              _login_id.c_str(),
              _local.ip_as_string.c_str(),
              _local.user_us_time_delta));
              
    // TODO maybe not show the whole IP but mask part of it?
    if (_local.display_id.empty()) {
        _local.display_id = ip_as_string();
        // Mask part of the IP
        std::transform(_local.display_id.begin(),
                       _local.display_id.end(),
                       _local.display_id.begin(),
                       ip_mask());
    }
}

const char *
user::status_to_string(int s) {
    switch (s) {
    case status_chatting: return langstr("status/chatting");
    case status_playing:  return langstr("status/playing");
    case status_away:     return langstr("status/away");
    }
    
    return langstr("status/unknown");
}

} // ns chat_gaming
