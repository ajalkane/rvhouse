#ifndef _CHAT_COMM_PDU_HEADER_H_
#define _CHAT_COMM_PDU_HEADER_H_

#include <boost/serialization/access.hpp>

namespace chat {
namespace comm {
namespace pdu {
    
    class header {
    public:
        inline header(int payload_type);
        
        inline int payload_type() const;
    private:
        friend class boost::serialization::access;
        
        int _payload_type;
        // When the class Archive corresponds to an output archive, the
        // & operator is defined similar to <<.  Likewise, when the class Archive
        // is a type of input archive the & operator is defined similar to >>.
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & _payload_type;
        }       
    };
    
    inline header::header(int ptype)
      : _payload_type(ptype)
    {}

    inline int 
    header::payload_type() const { return _payload_type; }
} // ns pdu
} // ns comm
} // ns chat

#endif //_CHAT_COMM_PDU_HEADER_H_
