#include "../main.h"
#include "house_app.h"
#include "watched_object.h"

namespace gui {

watched_object::watched_object(QObject *self) : _object_destroyed_sent(false), _self(self)
{
    ACE_DEBUG((LM_DEBUG, "watched_object::ctor\n"));
    app()->watched_object_created(_self);
    
}

watched_object::~watched_object() {
    ACE_DEBUG((LM_DEBUG, "watched_object::dtor\n"));
    _object_destroyed();
}

void
watched_object::_object_destroyed() {
    ACE_DEBUG((LM_DEBUG, "watched_object::_object_destroyed\n"));
    if (!_object_destroyed_sent) {
        ACE_DEBUG((LM_DEBUG, "watched_object::_object_destroyed: calling app watched_object_destroyed\n"));
        app()->watched_object_destroyed(_self);
        _object_destroyed_sent = true;
    }
}

} // ns gui
