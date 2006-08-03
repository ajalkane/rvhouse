#include "../main.h"
#include "house_app.h"
#include "watched_object.h"

namespace gui {

watched_object::watched_object() : _self(NULL)
{
    ACE_DEBUG((LM_DEBUG, "watched_object::ctor\n"));
    
}

watched_object::~watched_object() {
    ACE_DEBUG((LM_DEBUG, "watched_object::dtor, sending message\n"));
    if (_self)
        app()->handle(_self, FXSEL(SEL_COMMAND, house_app::ID_OBJECT_DESTROY), NULL);
}

void 
watched_object::create(FXObject *self)
{
    ACE_DEBUG((LM_DEBUG, "watched_object::create\n"));
    assert(self);
    _self = self;
    app()->handle(_self, FXSEL(SEL_COMMAND, house_app::ID_OBJECT_CREATE), NULL); 
}

} // ns gui
