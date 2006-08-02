#include "self.h"

#include "../messaging/messenger.h"
#include "../messaging/message_user.h"
#include "../messaging/message_room.h"
#include "../main.h"

namespace model {

// TODO randomize the sequence number,
// sequence number 0 is reserved for "unknown" sequence,
// which can be generated when for example when a new IP is
// noticed or a user is erased (by timeout)
self::self() : _seq(1), _user(NULL) {}

message *
self::user_as_message() {
    return new message_user(
               ::message::user,
               this->user(),
               this->sequence(),
               0
           );
}

void
self::user_send() {
    net_messenger()->send_msg(user_as_message());
}

message *
self::hosting_room_as_message() {
    return new message_room(
               ::message::room,
               this->hosting_room(),
               this->user().id(),
               this->sequence(),
               0
           );
}

void
self::hosting_room_send() {
    net_messenger()->send_msg(hosting_room_as_message());
}

void
self::clear() {
    _user = user_type(NULL);
    _room = room_type();
    _joining_room = room_type::id_type();
}

} // ns model
