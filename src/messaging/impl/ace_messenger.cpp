#include <ace/ACE.h>
#include <ace/Log_Msg.h>

#include "ace_messenger.h"

ace_messenger::ace_messenger(
    ACE_Reactor *r, ACE_Event_Handler *h, 
    ACE_Reactor_Mask m)
  : _reactor(r), _handler(h), _mask(m)
{
}

void
ace_messenger::wake_target() {
    ACE_DEBUG((LM_DEBUG, "ace_messenger: notifying ACE reactor from thread %t\n"));
    
    _reactor->notify(_handler); // , _mask);
}
