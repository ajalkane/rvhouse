#ifndef _CHAT_USER_H_
#define _CHAT_USER_H_

#include <string>

namespace chat {
template <class IdType = std::string, class RoomIdType = std::string>
class user {
public:
    typedef IdType     id_type;
    typedef RoomIdType room_id_type;

    inline user() {}
    inline user(const id_type &i) : _id(i) {}

    inline const id_type &id(const id_type &i) { return _id = i; }
    inline const id_type &id() const { return _id; }

    inline const room_id_type &room_id(const room_id_type &i) { return _room_id = i; }
    inline const room_id_type &room_id() const { return _room_id; }

    inline bool operator==(const user &o) const { return _id == o._id; }
    inline bool operator!=(const user &o) const { return !(*this == o); }
    inline bool operator<(const user &o) const  { return _id <  o._id; }

    inline user &operator=(const user &o) {
        // IdType is not copied as normally id is something that is
        // generated once and should not be changed for the object anymore
        // afterwards. If there is pressing need to change id it can be
        // set manually, or this behaviour overriden in the implementation
        // class.
        _room_id = o._room_id;
        return *this;
    }

protected:
    IdType     _id;
    RoomIdType _room_id;
};

} // ns chat
#endif //_CHAT_USER_H__
