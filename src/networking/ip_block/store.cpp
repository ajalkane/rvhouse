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
        add(*i);
    }
}

bool
store::add(uint32_t ip) {
    // some sanity checks
    if (ip != htonl(INADDR_ANY)      && 
        ip != htonl(INADDR_LOOPBACK) &&
        ip != htonl(INADDR_NONE)) {
        _store.insert(ip);
        return true;
    }
    return false;
}

} // ns ip_block
} // ns networking
