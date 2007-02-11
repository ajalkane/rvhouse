#include "store.h"

namespace networking {
namespace ip_block {

void
store::handle_message(message *msg) {
    message_block_users *m = dynamic_cast<message_block_users *>(msg);
    if (!m) return;
    
    message_block_users::list_type::const_iterator i   = m->ip_begin();
    message_block_users::list_type::const_iterator end = m->ip_end();
    for (; i != end; i++) {
        add(i->ip, i->mask);
    }
}

#if 0
bool
store::add(uint32_t ip) {
    // some sanity checks
    if (ip != INADDR_ANY      && 
        ip != INADDR_LOOPBACK &&
        ip != INADDR_NONE)
    {
        _store.insert(ip);
        return true;
    }
    return false;
}
#endif

bool
store::add(uint32_t ip, uint32_t mask) {
    // some sanity checks
    if (ip   != INADDR_ANY      && 
        ip   != INADDR_LOOPBACK &&
        ip   != INADDR_NONE     &&
        mask != INADDR_ANY) 
    {
        if (mask == INADDR_NONE) {
            _store.insert(ip);
        }
        else {
            ipmask ipandmask(ip & mask, mask);
            if (std::find(_ipmaskstore.begin(), _ipmaskstore.end(), ipandmask)
                == _ipmaskstore.end())
             {
                _ipmaskstore.push_back(ipandmask);
             } else {
                ACE_DEBUG((LM_DEBUG, "ip_block::add: duplicate masked ip\n"));
             }
        }
        return true;
    }
    
    return false;
}

} // ns ip_block
} // ns networking
