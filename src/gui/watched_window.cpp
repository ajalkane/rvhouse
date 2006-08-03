#include "../main.h"
#include "house_app.h"
#include "watched_window.h"

namespace gui {
    
watched_window::watched_window() : watched_object() // _self(NULL)
{
}

watched_window::~watched_window() {
    ACE_DEBUG((LM_DEBUG, "watched_window::dtor, sending message\n"));
//  if (_self)
//      app()->handle(_self, FXSEL(SEL_COMMAND, house_app::ID_WINDOW_DESTROY), NULL);
}

void 
watched_window::create(FXWindow *self)
{
    assert(_self == NULL);
    watched_object::create(self);
    //_self = self;
    //app()->handle(_self, FXSEL(SEL_COMMAND, house_app::ID_WINDOW_CREATE), NULL); 
}

} // ns gui
