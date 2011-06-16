#ifndef GUI_MODAL_DIALOG_GUARD_H_
#define GUI_MODAL_DIALOG_GUARD_H_

#include <string>

#include <QDialog>
#include <QWidget>

#include "message_handler.h"
#include "watched_object.h"

namespace gui {

/**
 * Classes inheriting from this class are guaranteed to be displayed only when possible.
 *
 * The concrete implementations must always be allocated from heap, not from stack. The objects
 * are automatically deleted when the window is closed.
 */
// IMPROVE Qt more descriptive name for the class might be something like modal_dialog_guarded or something
class modal_dialog_guard :
    public QDialog,
    public watched_object,
    public message_handler
{
protected:
    // Checks if a modal dialog can be displayed,
    // returns true if can, false otherwise...
    virtual bool can_display();

    virtual bool display_if_possible();

public:
    modal_dialog_guard(QWidget *parent);
    virtual ~modal_dialog_guard();
    // Returns true if was shown immediately
    virtual bool show_when_possible();

    void handle_message    (::message *msg);    
};

} // ns gui

#endif //GUI_MODAL_DIALOG_GUARD_H_
