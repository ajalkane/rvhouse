#ifndef _CHAT_GAMING_PDU_HEADER_H_
#define _CHAT_GAMING_PDU_HEADER_H_

#include <boost/serialization/access.hpp>

namespace chat_gaming {
namespace pdu {
    enum {
        id_message = 1,
        id_room_join,
        id_room_join_ack,
        id_room_join_full,
        id_room_join_refused,
        id_room_join_password,
        
        id_room_launch,
        id_room_kick,
        
        id_message_private,
        id_private_refused,
    };
        
    class header {
        unsigned    _sequence;      
        int _id;
        
        friend class boost::serialization::access;
        
        // When the class Archive corresponds to an output archive, the
        // & operator is defined similar to <<.  Likewise, when the class Archive
        // is a type of input archive the & operator is defined similar to >>.
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & _sequence;
            ar & _id;
        }       
        
    public: 
        inline header() : _sequence(0), _id(0) {}
        inline header(int d, unsigned s) : _sequence(s), _id(d) {}

        inline unsigned sequence() const     { return _sequence; }
        inline unsigned sequence(unsigned s) { return _sequence = s; }
        
        inline int id(int d)  { return _id = d; }
        inline int id() const { return _id; }
    };
} // ns pdu
} // ns chat_gaming

#endif //_CHAT_GAMING_PDU_HEADER_H_
