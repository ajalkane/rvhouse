#ifndef _CHAT_GAMING_PDU_ROOM_COMMAND_H_
#define _CHAT_GAMING_PDU_ROOM_COMMAND_H_

#include <boost/serialization/access.hpp>

#include <string>

#include "../room.h"
#include "../user.h"

namespace chat_gaming {
namespace pdu {
    class room_command {
        room::id_type  _room_id;
        user::id_type  _user_target;

        friend class boost::serialization::access;
        
        // When the class Archive corresponds to an output archive, the
        // & operator is defined similar to <<.  Likewise, when the class Archive
        // is a type of input archive the & operator is defined similar to >>.
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & _room_id;
            ar & _user_target;
        }       
        
    public: 
        inline room_command() {}
        inline room_command(const room::id_type &rid,
                            const user::id_type &target_uid)
            : _room_id(rid), _user_target(target_uid)
        {
            _room_id     = rid;
            _user_target = target_uid;
        }
        
        inline const room::id_type &room_id() const { return _room_id; }
        inline const room::id_type &room_id(room::id_type &rid) {
            return _room_id = rid;
        }

        inline const user::id_type &target_user_id() const { return _user_target; }
        inline const user::id_type &target_user_id(const user::id_type &uid) { 
            return _user_target = uid; 
        }
    };    
} // ns pdu
} // ns chat_gaming

#endif //_CHAT_GAMING_PDU_ROOM_COMMAND_H_
