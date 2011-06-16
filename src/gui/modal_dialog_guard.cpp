#include <QtGui>

#include "../main.h"
#include "../model/self.h"
#include "../chat_gaming/user.h"
#include "house_app.h"
#include "modal_dialog_guard.h"

namespace gui {


modal_dialog_guard::modal_dialog_guard(QWidget *parent) : QDialog(parent), watched_object(this) {
    ACE_DEBUG((LM_DEBUG, "modal_dialog_guard::ctor\n"));
    this->setModal(true);
    this->setAttribute(Qt::WA_DeleteOnClose);
}
    
modal_dialog_guard::~modal_dialog_guard() {
    ACE_DEBUG((LM_DEBUG, "modal_dialog_guard::dtor\n"));
}

bool 
modal_dialog_guard::can_display() {
    ACE_DEBUG((LM_DEBUG, "modal_dialog_guard::can_display\n"));
    // Qt doesn't seem to have problems displaying modal dialogs upon modal dialogs, so this code is commented out
    //    if (::app()->activeModalWidget()) {
    //        ACE_DEBUG((LM_DEBUG, "gui_dialog_guard::can_display: not because modal window\n"));
    //        return false;
    //    }
    if (self_model()->user().status() == chat_gaming::user::status_playing) {
        ACE_DEBUG((LM_DEBUG, "gui_dialog_guard::can_display: not because playing\n"));              
        return false;
    }
    ACE_DEBUG((LM_DEBUG, "gui_dialog_guard::can_display: yes\n"));
    return true;
}

bool
modal_dialog_guard::display_if_possible() {
    ACE_DEBUG((LM_DEBUG, "modal_dialog_guard::display_if_possible()\n"));
    if (can_display()) {
        this->show();
        return true;
    }
    return false;
}

bool
modal_dialog_guard::show_when_possible() {
    ACE_DEBUG((LM_DEBUG, "modal_dialog_guard::show_when_possible()\n"));
    return display_if_possible();
}

void
modal_dialog_guard::handle_message(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "modal_dialog_guard::handle_message\n"));  
    display_if_possible();
}

} // ns gui
