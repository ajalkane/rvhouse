#ifndef _CHAT_GAMING_PDU_ROOM_JOIN_H_
#define _CHAT_GAMING_PDU_ROOM_JOIN_H_

#include <boost/serialization/access.hpp>

#include <string>

#include "../room.h"
#include "../user.h"

namespace chat_gaming {
namespace pdu {
    class room_join {
        room::id_type  _room_id;
        std::string    _password;

        friend class boost::serialization::access;
        
        // When the class Archive corresponds to an output archive, the
        // & operator is defined similar to <<.  Likewise, when the class Archive
        // is a type of input archive the & operator is defined similar to >>.
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & _room_id;
            ar & _password;
        }       
        
    public: 
        inline room_join() {}
        inline room_join(const room &r) {
            _room_id  = r.id();
            _password = r.password();
        }
        
        inline const room::id_type &room_id() const { return _room_id; }
        inline const room::id_type &room_id(room::id_type &rid) {
            return _room_id = rid;
        }

        inline const std::string &password() const { return _password; }
        inline const std::string &password(std::string &p) {
            return _password = p;
        }
    };    
} // ns pdu
} // ns chat_gaming

#endif //_CHAT_GAMING_PDU_ROOM_JOIN_H_
