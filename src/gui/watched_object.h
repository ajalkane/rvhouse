#ifndef GUI_WATCHED_OBJECT_H_
#define GUI_WATCHED_OBJECT_H_

#include <QObject>

#include "../common.h"

namespace gui {
/**
 * Reports back stuff between main application <-> object of states
 */
class watched_object {
private:
    bool _object_destroyed_sent;
protected:
    QObject *_self;

    void _object_destroyed();
public:
    watched_object(QObject *self);
    virtual ~watched_object();
};

} // ns gui

#endif //GUI_WATCHED_OBJECT_H_
