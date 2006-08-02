#ifndef _HOUSE_MESSAGE_BOX_H_
#define _HOUSE_MESSAGE_BOX_H_

#include <fx.h>

#include "common.h"

#ifndef FXDIALOGBOX_H
#include "FXDialogBox.h"
#endif

/********************
 * This file has been copied from Fox's FXMessageBox as it
 * seems it is next to impossible to modify without doing
 * everything from the start. A pity... have to try and find
 * if there's a better way to overload the needed things.
 */

namespace FX {


/**
* A Message Box is a convenience class which provides a dialog for
* very simple common yes/no type interactions with the user.
* The message box has an optional icon, a title string, and the question
* which is presented to the user.  It also has up to three buttons which
* furnish standard responses to the question.
* Message boxes are usually ran modally: the question must be answered
* before the program may continue.
*/
class house_message_box : public FXDialogBox {
    FXDECLARE(house_message_box)
protected:
    house_message_box(){}
private:
    house_message_box(const house_message_box&);
    house_message_box &operator=(const house_message_box&);
    void initialize(const FXString& text,FXIcon* ic,FXuint whichbuttons, const FXString& url = "");
public:
    long onCmdClicked(FXObject*,FXSelector,void*);
    long onCmdCancel(FXObject*,FXSelector,void*);
public:
    enum{
        ID_CLICKED_YES=FXDialogBox::ID_LAST,
        ID_CLICKED_NO,
        ID_CLICKED_OK,
        ID_CLICKED_CANCEL,
        ID_CLICKED_QUIT,
        ID_CLICKED_SAVE,
        ID_CLICKED_SKIP,
        ID_CLICKED_SKIPALL,
        ID_LAST
    };
public:

    /// Construct message box with given caption, icon, and message text
    house_message_box(FXWindow* owner,const FXString& caption,const FXString& text,FXIcon* ic=NULL,FXuint opts=0,FXint x=0,FXint y=0);

    /// Construct free floating message box with given caption, icon, and message text
    house_message_box(FXApp* app,const FXString& caption,const FXString& text,FXIcon* ic=NULL,FXuint opts=0,FXint x=0,FXint y=0);

    house_message_box(FXApp* app,const FXString& url, const FXString& caption,const FXString& text,FXIcon* ic=NULL,FXuint opts=0,FXint x=0,FXint y=0);

    /**
    * Show a modal error message.
    * The text message may contain printf-tyle formatting commands.
    */
    static FXuint error(FXWindow* owner,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;

    /**
    * Show modal error message, in free floating window.
    */
    static FXuint error(FXApp* app,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;

    /**
    * Show a modal warning message
    * The text message may contain printf-tyle formatting commands.
    */
    static FXuint warning(FXWindow* owner,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;

    /**
    * Show modal warning message, in free floating window.
    */
    static FXuint warning(FXApp* app,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;

    /**
    * Show a modal question dialog
    * The text message may contain printf-tyle formatting commands.
    */
    static FXuint question(FXWindow* owner,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;

    /**
    * Show modal question message, in free floating window.
    */
    static FXuint question(FXApp* app,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;

    /**
    * Show a modal information dialog
    * The text message may contain printf-tyle formatting commands.
    */
    static FXuint information(FXWindow* owner,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;

    /**
    * Show modal information message, in free floating window.
    */
    static FXuint information(FXApp* app,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;

    static FXuint information(FXApp* app,FXuint opts,const char* url, const char* caption,const char* message,...) FX_PRINTF(5,6) ;

};

}

#endif // _HOUSE_MESSAGE_BOX_H_
