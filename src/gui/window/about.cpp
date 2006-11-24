#include <utility>
#include <string>
#include <list>
#include <sstream>

#include <fx.h>
#include <fxkeys.h>
#include "../component/FXLinkLabel.h"

#include "../../app_version.h"
#include "../../lang/manager.h"
#include "../../lang/info.h"
#include "../../lang/util.h"
#include "about.h"

namespace gui {
namespace window {

using component::FXLinkLabel;

FXIMPLEMENT(about, FXDialogBox, NULL, 0);

about::about(FXWindow *owner) 
    : FXDialogBox(owner, langstr("about_win/title"))
{
    std::ostringstream str;
    using std::endl;
    
    str << APP_NAME << " " << APP_VERSION << endl
        << endl
        << "Copyright (C) 2006 Arto Jalkanen" << endl
        << endl
        << langstr("about_win/license", "GPL") << endl
        << endl
        << langstr("about_win/icons_by", "ADX") << endl
        << langstr("about_win/house_icon_by", "Pigasque") << endl
        << endl
        << langstr("about_win/other_sites") << endl;
    new FXLabel(this, str.str().c_str(), NULL, JUSTIFY_LEFT);
    new FXLinkLabel(this, "http://www.rvhouse.net");
    new FXLinkLabel(this, "http://revolt.speedweek.net");
    new FXLinkLabel(this, "http://www.rvleague.atomysk.com");
    new FXLinkLabel(this, "http://www.rvtt.com");
    new FXLinkLabel(this, "http://www.revolt-cars.com");
    new FXLinkLabel(this, "http://www.frappr.com/revoltgame");
/*      << "http://revolt.speedweek.net" << endl
        << "http://rvleague.free.fr" << endl
        << "http://www.rvtt.com (french)" << endl
        << "http://www.revolt-cars.com (french)" << endl
        << "http://www.frappr.com/revoltgame" << endl */
    str.str("");
    str << endl
        << langstr("about_win/using") << endl
        << langstr("about_win/using_lib", 
                   "FOX C++ GUI Library", "Jeroen van der Zijp") << endl
        << langstr("about_win/using_lib", 
                   "ACE network library", "Douglas C. Schmidth") << endl
        << langstr("about_win/using_lib", 
                   "KadC P2P library", "Enzo Michelangeli") << endl
        << langstr("about_win/using_lib", 
                   "libdht, libreudp, libnetcomgrp", "ajalkane") << endl
        << langstr("about_win/using_lib", 
                   "Boost serialization", "Robert Ramey");

    std::list<lang::info> lang_infos = lang_mngr()->lang_infos();
    // Remove english, as its the original language and not a translation,
    // from the list before listing translations
    std::list<lang::info>::iterator new_end 
      = std::remove_if(lang_infos.begin(), lang_infos.end(), 
                       lang::info_match_lang_fobj("English"));
    lang_infos.erase(new_end, lang_infos.end());
    if (lang_infos.size() > 0) {
        str << endl << endl
            << langstr("about_win/translations") << endl;
        std::list<lang::info>::const_iterator i = lang_infos.begin();
        for (; i != lang_infos.end(); i++) {
            str << langstr("about_win/translation", 
                           i->lang().c_str(),
                           i->author().c_str(),
                           i->email().c_str(),
                           i->version().c_str())
                << endl; 
        }
    }                  
    new FXLabel(this, str.str().c_str(), NULL, JUSTIFY_LEFT);

    new FXSeparator(this);
    FXHorizontalFrame *bframe = new FXHorizontalFrame(this, LAYOUT_CENTER_X);

    new FXButton(
        bframe, langstr("common/ok_button"), NULL, this, ID_CANCEL,
        BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_CENTER_X|
        FRAME_RAISED|FRAME_THICK,
        0,0,0,0,20,20        
    );
    
    getAccelTable()->addAccel(MKUINT(KEY_F4,ALTMASK),this,FXSEL(SEL_COMMAND,ID_CANCEL));
}

void
about::create() {
    FXDialogBox::create();
}

} // ns window
} // ns gui
