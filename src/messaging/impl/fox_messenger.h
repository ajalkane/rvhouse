#ifndef _MESSAGING_FOX_MESSENGER_H_
#define _MESSAGING_FOX_MESSENGER_H_

#include <fx.h>
#include <FXGUISignal.h>

#include "../messenger.h"

/**
 * Implements a messenger for Fox toolkit
 * 
 * Only to be called from another thread to notify Fox's GUI thread.
 * The recipient of the message is responsible of calling delete on it.
 */
class fox_messenger : public messenger {
    FXApp       *_app;
    FXObject    *_target;
    FXSelector   _sel;
    FXGUISignal *_fox_signal;
    
protected:
    virtual void wake_target();
    
public:
    fox_messenger(FXApp *a, FXObject *target, FXSelector sel);
    virtual ~fox_messenger();
};

#endif //_MESSAGING_FOX_MESSENGER_H_
