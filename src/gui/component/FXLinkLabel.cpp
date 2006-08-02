#include "FXLinkLabel.h"

#include "../../common.h"
#include "../../main.h"
#include "../../executable/launcher.h"

namespace gui {
namespace component {

FXDEFMAP(FXLinkLabel) FXLinkLabelMap[]={
                                           FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXLinkLabel::onLeftBtnPress),
                                           FXMAPFUNC(SEL_TIMEOUT,FXLinkLabel::ID_TIMER,FXLinkLabel::onTimer),
                                       };
FXIMPLEMENT(FXLinkLabel,FXLabel,FXLinkLabelMap,ARRAYNUMBER(FXLinkLabelMap))


FXLinkLabel::FXLinkLabel(FXComposite* p,const FXString& text,FXIcon*
                         ic,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint
                         pt,FXint pb)
        :FXLabel(p,text,ic,opts,x,y,w,h,pl,pr,pt,pb){
    setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
    setTextColor(FXRGB(0,0,255));
}

FXLinkLabel::~FXLinkLabel(){
    getApp()->removeTimeout(this,ID_TIMER);
}

long FXLinkLabel::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
    ACE_DEBUG((LM_DEBUG, "FXLinkLabel::onLeftBtnPress\n"));
    FXString link = getTipText();
    if (!link.length()) link = getText();
    if(link.length()){
        ACE_DEBUG((LM_DEBUG, "FXLinkLabel::onLeftBtnPress2\n"));
        getApp()->beginWaitCursor();
        if (!launcher_file()->start(link.text())) {
            ACE_DEBUG((LM_DEBUG, "FXLinkLabel::onLeftBtnPress3\n"));
            getApp()->addTimeout(this,ID_TIMER,2000); // 2 seconds of away cursor
        } else{
            ACE_DEBUG((LM_DEBUG, "FXLinkLabel::onLeftBtnPress4\n"));
            getApp()->endWaitCursor();
            getApp()->beep();
        }
    }
    return 1;
}

long FXLinkLabel::onTimer(FXObject*,FXSelector,void* ptr){
    getApp()->endWaitCursor();
    return 1;
}

} // ns component
} // ns gui
