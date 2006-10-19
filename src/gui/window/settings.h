#ifndef _GUI_WINDOW_SETTINGS_H_
#define _GUI_WINDOW_SETTINGS_H_

#include <string>
#include <map>
#include <fx.h>

#include "../../common.h"
#include "../message_handler.h"
#include "../watched_window.h"

namespace gui {
namespace window {

class settings 
  : public FXMainWindow, 
    public message_handler, 
    public watched_window
{
    typedef FXMainWindow super;
    FXDECLARE(settings)
    
    FXFont *_font;
    FXFont *_titlefont;

    FXColor _base;
    FXColor _back;
    FXColor _border;
    FXColor _fore;
    FXColor _hilite;
    FXColor _shadow;
    FXColor _selfore;
    FXColor _selback;
    FXColor _tipfore;
    FXColor _tipback;
    FXColor _menufore;
    FXColor _menuback;

    FXDataTarget  _target_base;
    FXDataTarget  _target_back;
    FXDataTarget  _target_border;
    FXDataTarget  _target_fore;
    FXDataTarget  _target_hilite;
    FXDataTarget  _target_shadow;
    FXDataTarget  _target_selfore;
    FXDataTarget  _target_selback;
    FXDataTarget  _target_tipfore;
    FXDataTarget  _target_tipback;
    FXDataTarget  _target_menufore;
    FXDataTarget  _target_menuback;
    
   typedef std::map<std::string, FXCheckButton *> _check_map_type;
    _check_map_type _check_map;

    void _init();
    void _setup();
    void _pref_to_form();
    void _form_to_pref();
protected:
    settings() {}

public:
    enum {
        ID_COLORS = super::ID_LAST,
        ID_OK,
        ID_CHOOSE_FONT,
        ID_LAST,
    };
    
    settings(FXApp *a);
    ~settings();
    virtual void create();
    
    // long on_send_message(FXObject *from, FXSelector sel, void *);
    
    void handle_message    (::message *msg);
    long on_ok(FXObject *from, FXSelector sel, void *);    
    
};

} // ns window
} // ns gui

#endif //_GUI_WINDOW_SETTINGS_H_
