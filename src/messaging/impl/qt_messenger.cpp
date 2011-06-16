#include "qt_messenger.h"

qt_messenger::qt_messenger(QCoreApplication *a, QObject *target)
  : _app(a), _target(target)
{
}

qt_messenger::~qt_messenger() {
}
    
void
qt_messenger::wake_target() {
    ACE_DEBUG((LM_DEBUG, "qt_messenger: waking qt thread from thread %t\n"));
    
    _app->postEvent(_target, new qt_messenger_event());
}
