#ifndef _GUI_MODAL_DIALOG_GUARD_H_
#define _GUI_MODAL_DIALOG_GUARD_H_

#include <string>

#include <fx.h>

#include "message_handler.h"
#include "watched_object.h"

namespace gui {
    
class modal_dialog_guard : 
    public FXObject,
    public message_handler, 
    public watched_object
{
protected:
    FXDECLARE(modal_dialog_guard)

    // Checks if a modal dialog can be displayed,
    // returns true if can, false otherwise...
    virtual bool can_display();

    virtual bool display_if_possible();
    
    // Display is called when the dialog should be shown
    // Inherited classes must implement this
    // The class is deleted after display() returns.
    virtual void display();
public:
    modal_dialog_guard();
    virtual ~modal_dialog_guard();
    // Returns true if was shown immediately
    virtual bool show_when_possible();

    void handle_message    (::message *msg);    
};

} // ns gui

#endif //_GUI_MODAL_DIALOG_GUARD_H_
