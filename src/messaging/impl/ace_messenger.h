#ifndef _MESSAGING_ACE_MESSENGER_H_
#define _MESSAGING_ACE_MESSENGER_H_

#include <ace/Reactor.h>
#include <ace/Event_Handler.h>

#include "../messenger.h"

/**
 * Implements a messenger for ACE's reactor
 * 
 * The recipient of the message is responsible of calling delete on it.
 */
class ace_messenger : public messenger {
    ACE_Reactor       *_reactor;
    ACE_Event_Handler *_handler;
    ACE_Reactor_Mask   _mask;
    
protected:
    virtual void wake_target();
    
public:
    ace_messenger(ACE_Reactor *r, ACE_Event_Handler *h = NULL, 
                  ACE_Reactor_Mask m = ACE_Event_Handler::EXCEPT_MASK);
};

#endif //_MESSAGING_ACE_MESSENGER_H_
