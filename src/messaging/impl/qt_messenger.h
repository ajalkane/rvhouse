#ifndef MESSAGING_QT_MESSENGER_H_
#define MESSAGING_QT_MESSENGER_H_

#include <QApplication>

#include "../messenger.h"

/**
 * Implements a messenger for Qt toolkit
 * 
 * Only to be called from another thread to notify Qt's GUI thread.
 * The recipient of the message is responsible of calling delete on it.
 */
class qt_messenger : public messenger {
    QCoreApplication *_app;
    QObject          *_target;
    
protected:
    virtual void wake_target();
    
public:
    qt_messenger(QCoreApplication *a, QObject *target);
    virtual ~qt_messenger();
};

class qt_messenger_event : public QEvent {
public:
    // IMPROVE Qt should define the QEvent type in some meaningful way
    inline qt_messenger_event() : QEvent(QEvent::User) {}
};

#endif //MESSAGING_QT_MESSENGER_H_
