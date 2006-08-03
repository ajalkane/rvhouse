#ifndef _MESSAGING_MESSENGER_H_
#define _MESSAGING_MESSENGER_H_

#include <list>
#include <functional>
#include <fx.h>

#include "../common.h"
#include "message.h"

/**
 * Specifies an interface for a one way message queue.
 * 
 * The recipient of the message is responsible of calling delete on it.
 */
class messenger {
protected:
    typedef std::list<message *> _msgs_type;

    _msgs_type _msgs;
    FXMutex    _lock;

    virtual void wake_target() = 0;     
public:
    messenger();
    virtual ~messenger();
    
    // TODO should rather provide some sort of stream interface
    // for reading / writing messages
    template<typename OutIter> void collect_msgs(OutIter i);

    inline void send_msg(message *m);
};

template<typename OutIter> void
messenger::collect_msgs(OutIter i) {
    FXMutexLock guard(_lock);
    
    std::copy(_msgs.begin(),
              _msgs.end(),
              i);
    _msgs.clear();
}

inline void
messenger::send_msg(message *m) {
    ACE_DEBUG((LM_DEBUG, "messenger: locking for sending message to recipient\n"));
    FXMutexLock guard(_lock);

    ACE_DEBUG((LM_DEBUG, "messenger: appending to message queue\n"));
    
    _msgs.insert(_msgs.end(), m);
    wake_target();
}

#endif //_MESSAGING_MESSENGER_H_

