#ifndef _CHAT_ROOM_H_
#define _CHAT_ROOM_H_

#include <map>
#include <list>
#include <string>

namespace chat {
    template <class IdType = std::string, class OwnerIdType = std::string>
    class room {
    public:
        typedef IdType       id_type;
        typedef OwnerIdType  owner_id_type;
        
        inline room() {}
        inline room(const id_type &i) : _id(i) {}
        
        inline const id_type &id(const id_type &i) { return _id = i; }
        inline const id_type &id() const { return _id; }

        inline const owner_id_type &owner_id(const owner_id_type &i) { 
            return _owner_id = i; 
        }
        inline const owner_id_type &owner_id() const { return _owner_id; }

        inline bool operator==(const room &o) const { return _id == o._id; }
        inline bool operator<(const room &o) const  { return _id <  o._id; }    
        
        inline room &operator=(const room &o) {
            // IdType is not copied as normally id is something that is
            // generated once and should not be changed for the object anymore
            // afterwards. If there is pressing need to change id it can be
            // set manually, or this behaviour overriden in the implementation
            // class.
            _owner_id = o._owner_id;
            return *this;
        }
        
    protected:
        id_type       _id;
        owner_id_type _owner_id;
    };  
} // ns chat
#endif //_CHAT_ROOM_H_
