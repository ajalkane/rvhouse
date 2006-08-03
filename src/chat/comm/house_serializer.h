#ifndef _CHAT_COMM_HOUSE_SERIALIZER_H_
#define _CHAT_COMM_HOUSE_SERIALIZER_H_

#include <ostream>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "pdu/header.h"

namespace chat {
namespace comm {
    template <typename HouseType>
    class house_serializer {
    public:
        typedef HouseType house_type;
        typedef typename HouseType::user_type user_type;
        typedef typename HouseType::room_type room_type;

        typedef boost::archive::text_oarchive oarchive_type;
        
        void header(std::ostream &, int type, unsigned int version);
        void user_req(std::ostream &, const user_type &u);
    };

    template <typename HouseType> void
    house_serializer<HouseType>::header(
      std::ostream &os, const user_type &u
    ) {
        oarchive_type oa(os);
        
        os << pdu::header();
    }
    
    template <typename HouseType> void
    house_serializer<HouseType>::user_req(
      std::ostream &os, const user_type &u
    ) {
        oarchive_type oa(os);
        
        os << pdu::header();
    }
    
} // ns comm
} // ns chat

#endif //_CHAT_COMM_HOUSE_SERIALIZER_H_
