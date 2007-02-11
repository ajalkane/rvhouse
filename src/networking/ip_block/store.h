#ifndef _NETWORKING_IP_BLOCK_STORE_H_
#define _NETWORKING_IP_BLOCK_STORE_H_

#include <set>
#include <list>

#include <ace/INET_Addr.h>

#include "../../messaging/messenger.h"
#include "../../messaging/message.h"
#include "../../messaging/message_block_users.h"

#include <algorithm>

namespace networking {
namespace ip_block {

class store {
public:
    inline bool is_blocked(uint32_t ip) const {
        if (_ipmaskstore.size() == 0 && _store.size() == 0) return false;
        if (_store.count(ip) > 0) return true;

        _ipmaskstore_type::const_iterator i = _ipmaskstore.begin();                         
        for (; i != _ipmaskstore.end(); ++i) {
            uint32_t ipmasked = ip & i->mask;
            if (ipmasked == i->ip) return true;
        }
        return false;
    }

    // bool add(uint32_t ip);
    bool add(uint32_t ip, uint32_t mask = INADDR_NONE);
    
    void handle_message(message *m);
    
    inline void clear() { _store.clear(); _ipmaskstore.clear(); }
    inline size_t size() const { return _store.size() + _ipmaskstore.size(); }
private:
    struct ipmask {
        uint32_t ip;
        uint32_t mask;
        inline ipmask() : ip(0),mask(0) {}
        inline ipmask(uint32_t i, uint32_t m) : ip(i),mask(m) {}
        inline bool operator==(const ipmask &o) const {
            return ip == o.ip && mask == o.mask;
        }
    };
    typedef std::set<uint32_t> _store_type;
    typedef std::list<ipmask>  _ipmaskstore_type;
    _store_type  _store;
    _ipmaskstore_type _ipmaskstore;
};

} // ns ip_block
} // ns networking

#endif // _NETWORKING_IP_BLOCK_STORE_H_
