#include "fox_messenger.h"

fox_messenger::fox_messenger(FXApp *a, FXObject *target, FXSelector sel)
        : _app(a), _target(target), _sel(sel)
{
    _fox_signal = new FXGUISignal(a, target, sel, this);
}

fox_messenger::~fox_messenger() {
    delete _fox_signal;
}

void
fox_messenger::wake_target() {
    ACE_DEBUG((LM_DEBUG, "fox_messenger: waking fox thread from thread %t\n"));

    _fox_signal->signal();
}
