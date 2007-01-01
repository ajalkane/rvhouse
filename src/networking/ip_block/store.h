#ifndef _NETWORKING_IP_BLOCK_STORE_H_
#define _NETWORKING_IP_BLOCK_STORE_H_

#include <set>

#include "../../messaging/messenger.h"
#include "../../messaging/message.h"
#include "../../messaging/message_block_users.h"

namespace networking {
namespace ip_block {
    
class store {
public:
    inline bool is_blocked(uint32_t ip) const {
        if (_store.size() == 0) return false;
        return _store.count(ip) > 0;
    }

    bool add(uint32_t ip);
    
    void handle_message(message *m);
    
    inline void clear() { _store.clear(); }
private:
    typedef std::set<uint32_t> _store_type;
    _store_type _store;
};

} // ns ip_block
} // ns networking

#endif // _NETWORKING_IP_BLOCK_STORE_H_
