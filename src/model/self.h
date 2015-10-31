#ifndef _MODEL_SELF_H_
#define _MODEL_SELF_H_

#include <map>

#include "../common.h"
#include "../chat_gaming/user.h"
#include "../chat_gaming/room.h"

class message;

namespace model {

/**
 * Keeps a model of the self user... eg. what is his state and
 * what he wants it to be.
 */
class self {
    unsigned _seq;

public:
    typedef chat_gaming::room  room_type;
    typedef chat_gaming::user  user_type;
    
    self();
    
    // Returns an object to the self's information for manipulating and 
    // reference.
    inline user_type &user() { return _user; }
    message *user_as_message();
    void user_send();
    
    // If user is hosting, the information is kept here about the hosted
    // room.
    inline room_type &hosting_room() { return _room; }
    message *hosting_room_as_message();
    void hosting_room_send();
    
    inline void state_send() {
        if (hosting_room().id().empty() == false) hosting_room_send();
        user_send();
    }
    
    // If joining a room, the room_id is kept here.
    inline const room_type::id_type &joining_room() const { return _joining_room; }
    inline const room_type::id_type &joining_room(const room_type::id_type &i) { 
        return _joining_room = i; 
    }
    inline const bool &room_version(bool room_version) { return _room_version = room_version; }
    inline const bool &room_version() const { return _room_version; }
    inline const bool &room_version_all(bool room_version_all) { return _room_version_all = room_version_all; }
    inline const bool &room_version_all() const { return _room_version_all; }
    inline const bool &room_version_12_only(bool room_version_12_only) { return _room_version_12_only = room_version_12_only; }
    inline const bool &room_version_12_only() const { return _room_version_12_only; }
    inline const bool &room_version_rvgl(bool room_version_rvgl) { return _room_version_rvgl = room_version_rvgl; }
    inline const bool &room_version_rvgl() const { return _room_version_rvgl; }
    
    inline unsigned sequence() { 
        unsigned s = _seq++;
        // sequence number 0 is reserved for "unknown" sequence,
        // which can be generated when for example when a new IP is 
        // noticed or a user is erased (by timeout)
        if (_seq == 0) _seq = 1;
        return s; 
    }
    // Clears the state of the model
    void clear();
    
private:
    user_type _user;
    room_type _room;
    room_type::id_type _joining_room;
    bool _room_version;
    bool _room_version_all;
    bool _room_version_12_only;
    bool _room_version_rvgl;

};

} // ns model

#endif //_MODEL_SELF_H_
