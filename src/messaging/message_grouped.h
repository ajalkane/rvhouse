#ifndef _MESSAGING_MESSAGE_GROUPED_H_
#define _MESSAGING_MESSAGE_GROUPED_H_

#include "message.h"
#include "../model/house.h"

class message_grouped : public message {
    chat_gaming::user::id_type _sender_id;
    unsigned int               _seq; // Message sequence number
    int                        _group_base;
public: 
    // TODO would benefit from having message sender id here
    message_grouped(int msg_type, 
                    const chat_gaming::user::id_type &sender_id,
                    unsigned seq, 
                    int group_base);    
    virtual message *duplicate();
    
    inline int group_base() const { return _group_base; }
    inline unsigned sequence() const     { return _seq; }
    inline unsigned sequence(unsigned s) { return _seq = s; }
    inline const chat_gaming::user::id_type &sender_id() const { 
        return _sender_id;
    }
};

#endif //_MESSAGING_MESSAGE_GROUPED_H_
