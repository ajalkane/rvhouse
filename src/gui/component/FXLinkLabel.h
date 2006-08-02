#ifndef FX_LINK_LABEL_H_
#define FX_LINK_LABEL_H_

#include <fx.h>
#include "../../common.h"

namespace gui {
namespace component {

/** A label which points to a link.
* This class behaves very much like the FXLabel class but it
* points to some link which is started when clicked.
* 
* Author: Rafael de Pelegrini Soares?
*/

class FXLinkLabel : public FXLabel
{
protected:
    // FOX stuff
    FXDECLARE(FXLinkLabel)
    FXLinkLabel(){}
public:
    /// Constructor.
    FXLinkLabel(FXComposite* p,const FXString& text,FXIcon* ic=0,FXuint
                opts=LABEL_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint
                pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=0,FXint
                pb=0);

    /// Destructor.
    virtual ~FXLinkLabel();

    // FOX enum
    enum {
        ID_FIRST = FXLabel::ID_LAST,
        ID_TIMER,

        ID_LAST
    };

    // FOX messages
    long onLeftBtnPress(FXObject*,FXSelector,void*);
    long onTimer(FXObject*,FXSelector,void*);
};

} // ns component
} // ns gui

#endif /*FXLINKLABEL_H_*/
