/********************
 * This file has been copied from Fox's house_message_box as it
 * seems it is next to impossible to modify without doing
 * everything from the start. A pity... have to try and find
 * if there's a better way to overload the needed things.
 */

#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXHash.h"
#include "FXThread.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXApp.h"
#include "FXIcon.h"
#include "FXGIFIcon.h"
#include "FXSeparator.h"
#include "FXLabel.h"
#include "FXButton.h"
#include "FXHorizontalFrame.h"
#include "FXVerticalFrame.h"

#include "house_message_box.h"
#include "../component/FXLinkLabel.h"

using gui::component::FXLinkLabel;

// Contents of icons.h:
namespace FX {
    extern const unsigned char infoicon[];
    extern const unsigned char questionicon[];
    extern const unsigned char erroricon[];
    extern const unsigned char warningicon[];
}

// Padding for message box buttons
#define HORZ_PAD 30
#define VERT_PAD 2

#define MBOX_BUTTON_MASK   (MBOX_OK|MBOX_OK_CANCEL|MBOX_YES_NO|MBOX_YES_NO_CANCEL|MBOX_QUIT_CANCEL|MBOX_QUIT_SAVE_CANCEL|MBOX_SAVE_CANCEL_DONTSAVE)

using namespace FX;

/*******************************************************************************/

namespace FX {

// Map
FXDEFMAP(house_message_box) house_message_boxMap[]={
  FXMAPFUNC(SEL_COMMAND,house_message_box::ID_CANCEL,house_message_box::onCmdCancel),
  FXMAPFUNCS(SEL_COMMAND,house_message_box::ID_CLICKED_YES,house_message_box::ID_CLICKED_SKIPALL,house_message_box::onCmdClicked),
  };



// Object implementation
FXIMPLEMENT(house_message_box,FXDialogBox,house_message_boxMap,ARRAYNUMBER(house_message_boxMap))


// Construct message box with given caption, icon, and message text
house_message_box::house_message_box(FXWindow* owner,const FXString& caption,const FXString& text,FXIcon* ic,FXuint opts,FXint x,FXint y):
  FXDialogBox(owner,caption,opts|DECOR_TITLE|DECOR_BORDER,x,y,0,0, 0,0,0,0, 4,4){
  initialize(text,ic,opts&MBOX_BUTTON_MASK);
  }


// Construct free floating message box with given caption, icon, and message text
house_message_box::house_message_box(FXApp* a,const FXString& caption,const FXString& text,FXIcon* ic,FXuint opts,FXint x,FXint y):
  FXDialogBox(a,caption,opts|DECOR_TITLE|DECOR_BORDER,x,y,0,0, 0,0,0,0, 4,4){
  initialize(text,ic,opts&MBOX_BUTTON_MASK);
  }

// Construct free floating message box with given caption, icon, and message text
house_message_box::house_message_box(FXApp* a,const FXString &url, const FXString& caption,const FXString& text,FXIcon* ic,FXuint opts,FXint x,FXint y):
  FXDialogBox(a,caption,opts|DECOR_TITLE|DECOR_BORDER,x,y,0,0, 0,0,0,0, 4,4){
  initialize(text,ic,opts&MBOX_BUTTON_MASK, url);
  }


// Build contents
void house_message_box::initialize(const FXString& text,FXIcon* ic,FXuint whichbuttons, const FXString& url){
  FXButton *initial;
  FXVerticalFrame* content=new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXHorizontalFrame* info=new FXHorizontalFrame(content,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,10,10,10,10);
  new FXLabel(info,NULL,ic,ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXVerticalFrame* infocontent=new FXVerticalFrame(info,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(infocontent,text,NULL,JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  if (!url.empty()) {
    new FXLinkLabel(infocontent,url,NULL,JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  }
  
  new FXHorizontalSeparator(content,SEPARATOR_GROOVE|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X);
  FXHorizontalFrame* buttons=new FXHorizontalFrame(content,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH,0,0,0,0,10,10,5,5);
  if(whichbuttons==MBOX_OK){
    initial=new FXButton(buttons,langstr("message_box/ok"),NULL,this,ID_CLICKED_OK,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    initial->setFocus();
    }
  else if(whichbuttons==MBOX_OK_CANCEL){
    initial=new FXButton(buttons,langstr("message_box/ok"),NULL,this,ID_CLICKED_OK,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,langstr("message_box/cancel"),NULL,this,ID_CLICKED_CANCEL,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    initial->setFocus();
    }
  else if(whichbuttons==MBOX_YES_NO){
    initial=new FXButton(buttons,langstr("message_box/yes"),NULL,this,ID_CLICKED_YES,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,langstr("message_box/no"),NULL,this,ID_CLICKED_NO,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    initial->setFocus();
    }
  else if(whichbuttons==MBOX_YES_NO_CANCEL){
    initial=new FXButton(buttons,langstr("message_box/yes"),NULL,this,ID_CLICKED_YES,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,langstr("message_box/no"),NULL,this,ID_CLICKED_NO,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,langstr("message_box/cancel"),NULL,this,ID_CLICKED_CANCEL,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    initial->setFocus();
    }
  else if(whichbuttons==MBOX_QUIT_CANCEL){
    initial=new FXButton(buttons,langstr("message_box/quit"),NULL,this,ID_CLICKED_QUIT,BUTTON_INITIAL|BUTTON_DEFAULT|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,langstr("message_box/cancel"),NULL,this,ID_CLICKED_CANCEL,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    initial->setFocus();
    }
  else if(whichbuttons==MBOX_QUIT_SAVE_CANCEL){
    new FXButton(buttons,langstr("message_box/quit"),NULL,this,ID_CLICKED_QUIT,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    initial=new FXButton(buttons,"&Save",NULL,this,ID_CLICKED_SAVE,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,langstr("message_box/cancel"),NULL,this,ID_CLICKED_CANCEL,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    initial->setFocus();
    }
  else if(whichbuttons==MBOX_SKIP_SKIPALL_CANCEL){
    initial=new FXButton(buttons,"&Skip",NULL,this,ID_CLICKED_SKIP,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,"Skip &All",NULL,this,ID_CLICKED_SKIPALL,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,langstr("message_box/cancel"),NULL,this,ID_CLICKED_CANCEL,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    initial->setFocus();
    }
  else if (whichbuttons==MBOX_SAVE_CANCEL_DONTSAVE){
    buttons->setPackingHints(PACK_NORMAL);
    new FXButton(buttons,"&Don't Save",NULL,this,ID_CLICKED_NO,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_CENTER_X,0,0,0,0,15,15,VERT_PAD,VERT_PAD);
    FXHorizontalFrame *buttons3=new FXHorizontalFrame(buttons,LAYOUT_RIGHT|PACK_UNIFORM_WIDTH,0,0,0,0,0,0,0,0);
    new FXButton(buttons3,langstr("message_box/cancel"),NULL,this,ID_CLICKED_CANCEL,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,15,15,VERT_PAD,VERT_PAD);
    initial=new FXButton(buttons3,"&Save",NULL,this,ID_CLICKED_YES,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,15,15,VERT_PAD,VERT_PAD);
    initial->setFocus();
    }
  }


// Close dialog with a cancel
long house_message_box::onCmdClicked(FXObject*,FXSelector sel,void*){
  getApp()->stopModal(this,MBOX_CLICKED_YES+(FXSELID(sel)-ID_CLICKED_YES));
  hide();
  return 1;
  }


// Close dialog with a cancel
long house_message_box::onCmdCancel(FXObject* sender,FXSelector,void* ptr){
  return house_message_box::onCmdClicked(sender,FXSEL(SEL_COMMAND,ID_CLICKED_CANCEL),ptr);
  }


/*******************************************************************************/

// Show a modal error message
FXuint house_message_box::error(FXWindow* owner,FXuint opts,const char* caption,const char* message,...){
  FXGIFIcon icon(owner->getApp(),erroricon);
  va_list arguments;
  va_start(arguments,message);
  house_message_box box(owner,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
  va_end(arguments);
  return box.execute(PLACEMENT_OWNER);
  }


// Show a modal error message, in free floating window
FXuint house_message_box::error(FXApp* app,FXuint opts,const char* caption,const char* message,...){
  FXGIFIcon icon(app,erroricon);
  va_list arguments;
  va_start(arguments,message);
  house_message_box box(app,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
  va_end(arguments);
  return box.execute(PLACEMENT_SCREEN);
  }



// Show a modal warning message
FXuint house_message_box::warning(FXWindow* owner,FXuint opts,const char* caption,const char* message,...){
  FXGIFIcon icon(owner->getApp(),warningicon);
  va_list arguments;
  va_start(arguments,message);
  house_message_box box(owner,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
  va_end(arguments);
  return box.execute(PLACEMENT_OWNER);
  }


// Show a modal warning message, in free floating window
FXuint house_message_box::warning(FXApp* app,FXuint opts,const char* caption,const char* message,...){
  FXGIFIcon icon(app,warningicon);
  va_list arguments;
  va_start(arguments,message);
  house_message_box box(app,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
  va_end(arguments);
  return box.execute(PLACEMENT_SCREEN);
  }


// Show a modal question dialog
FXuint house_message_box::question(FXWindow* owner,FXuint opts,const char* caption,const char* message,...){
  FXGIFIcon icon(owner->getApp(),questionicon);
  va_list arguments;
  va_start(arguments,message);
  house_message_box box(owner,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
  va_end(arguments);
  return box.execute(PLACEMENT_OWNER);
  }


// Show a modal question dialog, in free floating window
FXuint house_message_box::question(FXApp* app,FXuint opts,const char* caption,const char* message,...){
  FXGIFIcon icon(app,questionicon);
  va_list arguments;
  va_start(arguments,message);
  house_message_box box(app,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
  va_end(arguments);
  return box.execute(PLACEMENT_SCREEN);
  }


// Show a modal information dialog
FXuint house_message_box::information(FXWindow* owner,FXuint opts,const char* caption,const char* message,...){
  FXGIFIcon icon(owner->getApp(),infoicon);
  va_list arguments;
  va_start(arguments,message);
  house_message_box box(owner,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
  va_end(arguments);
  return box.execute(PLACEMENT_OWNER);
  }


// Show a modal information dialog, in free floating window
FXuint house_message_box::information(FXApp* app,FXuint opts,const char* caption,const char* message,...){
  FXGIFIcon icon(app,infoicon);
  va_list arguments;
  va_start(arguments,message);
  house_message_box box(app,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
  va_end(arguments);
  return box.execute(PLACEMENT_SCREEN);
  }

FXuint house_message_box::information(FXApp* app,FXuint opts,const char *url, const char* caption,const char* message,...){
  FXGIFIcon icon(app,infoicon);
  va_list arguments;
  va_start(arguments,message);
  house_message_box box(app,url,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
  va_end(arguments);
  return box.execute(PLACEMENT_SCREEN);
  }
  
} // namespace FX 
