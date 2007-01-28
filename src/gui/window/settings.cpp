#include <utility>

#include <fx.h>
#include <fxkeys.h>

#include "../../messaging/message_grouped.h"
#include "../../icon_store.h"
#include "../../util.h"
#include "../util/util.h"
#include "../house_app.h"
#include "settings.h"

namespace gui {
namespace window {

namespace {
    const char *general_check_order[] = {
        "flash_main_chat",
        // "flash_room_chat",
        "flash_new_room",
        // "away_minimize"
        "flash_new_user",
        "flash_nick",
        "global_ignore",
    };
    bool general_check_defaults[] = {
        false,
        // false,
        false,
        // true,
        false,
        false,
        true,
    };
}

FXDEFMAP(settings) settings_map[]= {
  FXMAPFUNC(SEL_COMMAND,  settings::ID_OK, 
                          settings::on_ok),
};

// FXIMPLEMENT(settings, settings::super, settings_map, ARRAYNUMBER(settings_map))
FXIMPLEMENT(settings, settings::super, settings_map, ARRAYNUMBER(settings_map))

settings::settings(FXApp *a)
    : super(a, "", NULL, NULL, DECOR_ALL, 0, 0, 350, 350),
    _font(NULL),
    _titlefont(NULL)
     // , 0,0,0,0,0,0),
{
    _init();
}

void
settings::_init() {
    setIcon(app_icons()->get("rv_house"));
    setMiniIcon(app_icons()->get("rv_house"));
    setTitle(langstr("settings_win/title"));
    
    /// Retrieve Current Color Settings
    _base      = getApp()->getBaseColor();
    _back      = getApp()->getBackColor();
    _border    = getApp()->getBorderColor();
    _fore      = getApp()->getForeColor();
    _hilite    = getApp()->getHiliteColor();
    _shadow    = getApp()->getShadowColor();
    _selfore   = getApp()->getSelforeColor();
    _selback   = getApp()->getSelbackColor();
    _tipfore   = getApp()->getTipforeColor();
    _tipback   = getApp()->getTipbackColor();
    _menufore  = getApp()->getSelMenuTextColor();
    _menuback  = getApp()->getSelMenuBackColor();

    /// Setup the Datatargets
    _target_base.connect(_base);
    _target_back.connect(_back);
    _target_border.connect(_border);
    _target_fore.connect(_fore);
    _target_hilite.connect(_hilite);
    _target_shadow.connect(_shadow);
    _target_selfore.connect(_selfore);
    _target_selback.connect(_selback);
    _target_tipfore.connect(_tipfore);
    _target_tipback.connect(_tipback);
    _target_menufore.connect(_menufore);
    _target_menuback.connect(_menuback);

    _target_base.setTarget(this);
    _target_back.setTarget(this);
    _target_border.setTarget(this);
    _target_fore.setTarget(this);
    _target_hilite.setTarget(this);
    _target_shadow.setTarget(this);
    _target_selfore.setTarget(this);
    _target_selback.setTarget(this);
    _target_tipfore.setTarget(this);
    _target_tipback.setTarget(this);
    _target_menufore.setTarget(this);
    _target_menuback.setTarget(this);
    
    _target_base.setSelector(ID_COLORS);
    _target_back.setSelector(ID_COLORS);
    _target_border.setSelector(ID_COLORS);
    _target_fore.setSelector(ID_COLORS);
    _target_hilite.setSelector(ID_COLORS);
    _target_shadow.setSelector(ID_COLORS);
    _target_selfore.setSelector(ID_COLORS);
    _target_selback.setSelector(ID_COLORS);
    _target_tipfore.setSelector(ID_COLORS);
    _target_tipback.setSelector(ID_COLORS);
    _target_menufore.setSelector(ID_COLORS);
    _target_menuback.setSelector(ID_COLORS);

    _setup();
    _pref_to_form();
        
    getAccelTable()->addAccel(MKUINT(KEY_F4,ALTMASK),this,FXSEL(SEL_COMMAND,ID_CLOSE));    
}

settings::~settings() {
    ACE_DEBUG((LM_DEBUG, "settings: dtor\n"));
    
    delete _font;
    delete _titlefont;    
}

long
settings::on_ok(FXObject *from, FXSelector sel, void *ptr) {
    ACE_DEBUG((LM_DEBUG, "settings: on_ok\n"));
    _form_to_pref();
    return this->handle(from, FXSEL(FXSELTYPE(sel), ID_CLOSE), ptr);
}

void
settings::create() {
    ACE_DEBUG((LM_DEBUG, "settings::create this %d\n", this));
    super::create();
    watched_window::create(this);
    show(PLACEMENT_SCREEN);
}

void
settings::_setup() {
    /**
     * Much of this code is taken from Fox's ControlPanel example
     */
    FXFontDesc fontdescription;
    getApp()->getNormalFont()->create();
    getApp()->getNormalFont()->getFontDesc(fontdescription);
  
    _font = new FXFont(getApp(),fontdescription);
    _font->create();
  
    fontdescription.size = (FXuint) (((double)fontdescription.size) * 1.5);
    _titlefont = new FXFont(getApp(),fontdescription);
    _titlefont->create();
  
    FXHorizontalFrame *hframe=NULL;
    FXVerticalFrame   *frame=NULL;
    FXVerticalFrame   *vframe=NULL;
    FXMatrix          *matrix=NULL;
    FXLabel           *label=NULL;
    FXColorWell       *colorwell=NULL;
    FXSpinner         *spinner=NULL;
    FXCheckButton     *check=NULL;
    
    FXVerticalFrame *vmain = new FXVerticalFrame(
        this,
        LAYOUT_FILL_X|LAYOUT_FILL_Y,
        0,0,0,0,0,0,0,0,0,0
    );
  
    FXIcon *desktopicon = app_icons()->get("rv_house");

    // Assuming we have a icon of size 48x48, using different
    // spacing will give us about the same size header.
    const FXint spacing=(desktopicon ? 5 : 15);
  
    // Create nice header
    label = new FXLabel(
        vmain,langstr("settings_win/title"),desktopicon,
        LAYOUT_FILL_X|JUSTIFY_LEFT|TEXT_AFTER_ICON,
        0,0,0,0,spacing,spacing,spacing,spacing
    );
    label->setBackColor(FXRGB(255,255,255));
    label->setTextColor(FXRGB(  0,  0,  0));
    label->setFont(_titlefont);
  
    new FXSeparator(vmain,SEPARATOR_GROOVE|LAYOUT_FILL_X);
  
    FXHorizontalFrame *hmainframe = new FXHorizontalFrame(
        vmain,
        LAYOUT_FILL_X|LAYOUT_FILL_Y,
        0,0,0,0,0,0,0,0,0,0
    );
  
    FXVerticalFrame *buttonframe = new FXVerticalFrame(
        hmainframe,
        LAYOUT_FILL_Y|LAYOUT_LEFT|PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT,
        0,0,0,0, 
        DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING
    );
  
    new FXSeparator(hmainframe,SEPARATOR_GROOVE|LAYOUT_FILL_Y);
  
    FXSwitcher *switcher = new FXSwitcher(
        hmainframe,
        LAYOUT_FILL_X|LAYOUT_FILL_Y,
        0,0,0,0,0,0,0,0
    );
  
    vframe = new FXVerticalFrame(buttonframe,FRAME_SUNKEN,0,0,0,0,0,0,0,0);
    new FXButton(
        vframe,langstr("settings_win/general_tab"),NULL,switcher,
        FXSwitcher::ID_OPEN_FIRST,
        FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL
    );

    vframe = new FXVerticalFrame(buttonframe,FRAME_SUNKEN,0,0,0,0,0,0,0,0);
    new FXButton(
        vframe,langstr("settings_win/colors_tab"),NULL,switcher,
        FXSwitcher::ID_OPEN_SECOND,
        FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL
    );

    /*****************************************
     * Start of General view
     */
  
    vframe = new FXVerticalFrame(
        switcher,
        LAYOUT_FILL_X|LAYOUT_FILL_Y,
        0,0,0,0,0,0,0,0,0,0
    );
  
    new FXSeparator(vframe,SEPARATOR_GROOVE|LAYOUT_FILL_X);
    
    for (size_t i = 0; i < array_sizeof(general_check_order); i++) {
        const char *key     = general_check_order[i];
        std::string langkey = "settings_win/";
        langkey += key;
        _check_map[key] = new FXCheckButton(vframe, langstr(langkey.c_str()));
    }

    /*****************************************
     * Start of Themes view
     */
    vframe = new FXVerticalFrame(
        switcher,
        LAYOUT_FILL_X|LAYOUT_FILL_Y,
        0,0,0,0,0,0,0,0,0,0
    );
  
    hframe = new FXHorizontalFrame(
        vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y,
        0,0,0,0,0,0,0,0,0,0
    );
    new FXSeparator(vframe,SEPARATOR_GROOVE|LAYOUT_FILL_X);
  
  
    frame  = new FXVerticalFrame(hframe,LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,0);
    new FXSeparator(hframe,SEPARATOR_GROOVE|LAYOUT_FILL_Y);

#if 0  
    FXVerticalFrame *themeframe = new FXVerticalFrame(frame,LAYOUT_FILL_X,0,0,0,0,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING);
    new FXLabel(themeframe,"Theme: ",NULL,LAYOUT_CENTER_Y);
    list = new FXListBox(themeframe,this,ID_COLOR_THEME,LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK);
    list->setNumVisible(9);
    initColors();
#endif  
  
    new FXSeparator(frame,SEPARATOR_GROOVE|LAYOUT_FILL_X);
  
    matrix = new FXMatrix(
        frame,2,
        LAYOUT_FILL_Y|MATRIX_BY_COLUMNS,
        0,0,0,0,DEFAULT_SPACING,
        DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,1,1
    );
  
    colorwell = new FXColorWell(matrix,FXRGB(0,0,255),&_target_base,FXDataTarget::ID_VALUE);
    label     = new FXLabel(matrix,langstr("settings_win/base_color"));
  
    colorwell = new FXColorWell(matrix,FXRGB(0,0,255),&_target_border,FXDataTarget::ID_VALUE);
    label     = new FXLabel(matrix,langstr("settings_win/border_color"));
  
    colorwell = new FXColorWell(matrix,FXRGB(0,0,255),&_target_fore,FXDataTarget::ID_VALUE);
    label     = new FXLabel(matrix,langstr("settings_win/text_color"));
  
    colorwell = new FXColorWell(matrix,FXRGB(0,0,255),&_target_back,FXDataTarget::ID_VALUE);
    label     = new FXLabel(matrix,langstr("settings_win/bg_color"));
  
    colorwell = new FXColorWell(matrix,FXRGB(0,0,255),&_target_selfore,FXDataTarget::ID_VALUE);
    label     = new FXLabel(matrix,langstr("settings_win/sel_text_color"));
  
    colorwell = new FXColorWell(matrix,FXRGB(0,0,255),&_target_selback,FXDataTarget::ID_VALUE);
    label     = new FXLabel(matrix,langstr("settings_win/sel_bg_color"));
  
    colorwell = new FXColorWell(matrix,FXRGB(0,0,255),&_target_menufore,FXDataTarget::ID_VALUE);
    label     = new FXLabel(matrix,langstr("settings_win/sel_mt_color"));
  
    colorwell = new FXColorWell(matrix,FXRGB(0,0,255),&_target_menuback,FXDataTarget::ID_VALUE);
    label     = new FXLabel(matrix,langstr("settings_win/sel_mbg_color"));
  
    colorwell = new FXColorWell(matrix,FXRGB(0,0,255),&_target_tipfore,FXDataTarget::ID_VALUE);
    label     = new FXLabel(matrix,langstr("settings_win/tip_text_color"));
  
    colorwell = new FXColorWell(matrix,FXRGB(0,0,255),&_target_tipback,FXDataTarget::ID_VALUE);
    label     = new FXLabel(matrix,langstr("settings_win/tip_bg_color"));

#if 0  
    frame = new FXVerticalFrame(hframe,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,0,0);
  
    tabbook = new FXTabBook(frame,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,0,0,0,0);
    tabitem  = new FXTabItem(tabbook," Item 1 ");
    tabframe = new FXVerticalFrame(tabbook,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_THICK|FRAME_RAISED);
  
    labeltextframe1 = new FXHorizontalFrame(tabframe,LAYOUT_FILL_X);
    label1 = new FXLabel(labeltextframe1,"Label with Text",NULL);
    textfield1 = new FXTextField(labeltextframe1,30,NULL,0,LAYOUT_FILL_X|FRAME_THICK|FRAME_SUNKEN);
    textfield1->setText("Select this text, to see the selected colors");
  
    labeltextframe2 = new FXHorizontalFrame(tabframe,LAYOUT_FILL_X);
    textframe1 = new FXHorizontalFrame(labeltextframe2,LAYOUT_FILL_X|FRAME_THICK|FRAME_SUNKEN,0,0,0,0,2,2,2,2,0,0);
    label3 = new FXLabel(textframe1,"Selected Text (with focus)",NULL,LAYOUT_FILL_X,0,0,0,0,1,1,1,1);
    textframe2 = new FXHorizontalFrame(labeltextframe2,LAYOUT_FILL_X|FRAME_THICK|FRAME_SUNKEN,0,0,0,0,2,2,2,2,0,0);
    label4 = new FXLabel(textframe2,"Selected Text (no focus)",NULL,LAYOUT_FILL_X,0,0,0,0,1,1,1,1);
  
    sep1 = new FXSeparator(tabframe,LAYOUT_FILL_X|SEPARATOR_LINE);
  
    tabsubframe = new FXHorizontalFrame(tabframe,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  
    grpbox1 = new FXGroupBox(tabsubframe,"MenuPane",FRAME_GROOVE|LAYOUT_FILL_Y|LAYOUT_FILL_X);
  
    menuframe = new FXVerticalFrame(grpbox1,FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X|LAYOUT_CENTER_Y,0,0,0,0,0,0,0,0,0,0);
    menulabels[0]=new FXLabel(menuframe,"&Open",NULL,LABEL_NORMAL,0,0,0,0,16,4);
    menulabels[1]=new FXLabel(menuframe,"S&ave",NULL,LABEL_NORMAL,0,0,0,0,16,4);
    sep2 = new FXSeparator(menuframe,LAYOUT_FILL_X|SEPARATOR_GROOVE);
    menulabels[2]=new FXLabel(menuframe,"I&mport",NULL,LABEL_NORMAL,0,0,0,0,16,4);
    menulabels[4]=new FXLabel(menuframe,"Selected Menu Entry",NULL,LABEL_NORMAL,0,0,0,0,16,4);
    menulabels[3]=new FXLabel(menuframe,"Print",NULL,LABEL_NORMAL,0,0,0,0,16,4);
    sep3 = new FXSeparator(menuframe,LAYOUT_FILL_X|SEPARATOR_GROOVE);
    menulabels[5]=new FXLabel(menuframe,"&Quit",NULL,LABEL_NORMAL,0,0,0,0,16,4);
  
    grpbox2 = new FXGroupBox(tabsubframe,"Tooltips",FRAME_GROOVE|LAYOUT_FILL_Y|LAYOUT_FILL_X);
  
    label2 = new FXLabel(grpbox2,"Sample Tooltip",NULL,FRAME_LINE|LAYOUT_CENTER_X);
    label5 = new FXLabel(grpbox2,"Multiline Sample\n Tooltip",NULL,FRAME_LINE|LAYOUT_CENTER_X);
  
    hframe = new FXHorizontalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING);
  
    new FXLabel(hframe,"Normal Font: ",NULL,LAYOUT_CENTER_Y);
    fontbutton = new FXButton(hframe," ",NULL,this,ID_CHOOSE_FONT,LAYOUT_CENTER_Y|FRAME_RAISED|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_X);
  
  
    vframe = new FXVerticalFrame(switcher,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,0);
  
    hframe = new FXHorizontalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING);
    new FXLabel(hframe,"Icon Search Path",NULL,LAYOUT_CENTER_Y);
    new FXTextField(hframe,2,&target_iconpath,FXDataTarget::ID_VALUE,LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK);
  
    new FXSeparator(vframe,SEPARATOR_GROOVE|LAYOUT_FILL_X);
  
  
    hframe = new FXHorizontalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,0);
    vframe = new FXVerticalFrame(hframe,LAYOUT_FILL_Y,0,0,0,0,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING);
    new FXSeparator(hframe,SEPARATOR_GROOVE|LAYOUT_FILL_Y);
  
  
    new FXLabel(vframe,"File Binding: ");
    frame = new FXVerticalFrame(vframe,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0,0,0,0,0);
  
    filebindinglist = new FXList(frame,this,ID_SELECT_FILEBINDING,LAYOUT_FILL_Y|LAYOUT_FILL_X|LIST_BROWSESELECT);
    filebindinglist->setSortFunc(FXList::ascending);
    FXHorizontalFrame * listbuttonframe = new FXHorizontalFrame(vframe,PACK_UNIFORM_WIDTH|LAYOUT_FILL_X,0,0,0,0,0,0,0,0);
    new FXButton(listbuttonframe,"&New\tAdd New Binding",NULL,this,ID_CREATE_FILEBINDING);
    new FXButton(listbuttonframe,"&Delete\tDelete Selected Binding",NULL,this,ID_REMOVE_FILEBINDING);
    new FXButton(listbuttonframe,"&Edit\tEdit Binding Name",NULL,this,ID_RENAME_FILEBINDING);
#endif


    new FXSeparator(vmain,SEPARATOR_GROOVE|LAYOUT_FILL_X);
    FXHorizontalFrame *closebox=new FXHorizontalFrame(
        vmain,
        LAYOUT_CENTER_X);

    util::create_default_button(
        closebox, langstr("common/ok_button"), this, ID_OK
    );
    util::create_button(
        closebox, langstr("common/cancel_button"), this, ID_CLOSE
    );
    
}

void
settings::_pref_to_form() {
    for (size_t i = 0; i < array_sizeof(general_check_order); i++) {
        const char *key = general_check_order[i];
        bool    def_val = general_check_defaults[i];
        bool        val = pref()->get<bool>("general", key, def_val);
        _check_map[key]->setCheck(val);
    }    
}

void 
settings::_form_to_pref() {
    for (size_t i = 0; i < array_sizeof(general_check_order); i++) {
        const char *key = general_check_order[i];
        bool        val = _check_map[key]->getCheck() ? true : false;
        pref()->set<bool>("general", key, val);
    }

    getApp()->setBaseColor(_base);
    getApp()->setBackColor(_back);
    getApp()->setBorderColor(_border);
    getApp()->setForeColor(_fore);
    getApp()->setHiliteColor(_hilite);
    getApp()->setShadowColor(_shadow);
    getApp()->setSelforeColor(_selfore);
    getApp()->setSelbackColor(_selback);
    getApp()->setTipforeColor(_tipfore);
    getApp()->setTipbackColor(_tipback);
    getApp()->setSelMenuTextColor(_menufore);
    getApp()->setSelMenuBackColor(_menuback);
    
    pref()->save();
    app_opts.init();    
}

void
settings::handle_message(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "settings::handle_message\n"));
    
    message_grouped *mg = dynamic_cast<message_grouped *>(msg);
    if (!mg) return;
}


} // ns window
} // ns gui
