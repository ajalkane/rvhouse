#include "client_nop.h"

namespace networking {
namespace reporter {

client_nop::client_nop()
{
    
}

client_nop::~client_nop() {
}

void 
client_nop::user_self(const chat_gaming::user &s) {
}

void
client_nop::new_user(const chat_gaming::user &u, int grp) {
}

void 
client_nop::user_updated(const chat_gaming::user &oldu,
                     const chat_gaming::user &newu, int grp)
{
}

void 
client_nop::user_removed(const chat_gaming::user &u, int grp) {
}

void 
client_nop::connected(int grp) {
}

void 
client_nop::disconnected(int grp) {
}

void 
client_nop::disabled(int grp) {
}

void 
client_nop::dht_connected() {
}

void 
client_nop::dht_bootstrap() {
}

void 
client_nop::dht_disconnected() {
}

void 
client_nop::dht_ip_found(const ACE_INET_Addr &addr) {
}

void 
client_nop::ext_ip_detected(const std::string &ipstr) {
}

void 
client_nop::ext_ip_failed() {
}

} // ns reporter
} // ns networking

