#include <fx.h>

#include "house_tree_item.h"

namespace gui {
namespace component {

FXIMPLEMENT(house_tree_item, FXTreeItem, NULL, 0)


house_tree_item::house_tree_item(
) : _color(FXRGB(0,0,0))
{
}

// Copied from Fox Toolkit sources and modified to support what we need.
#define ICON_SPACING        4   // Spacing between parent and child in x direction
#define TEXT_SPACING        4   // Spacing between icon and text
#define SIDE_SPACING        4   // Spacing between side and item
#define DEFAULT_INDENT      8   // Indent between parent and child
#define HALFBOX_SIZE        4   // Half box size
#define BOX_FUDGE           3   // Fudge border around box

void 
house_tree_item::draw(const FXTreeList* list,FXDC& dc,FXint xx,FXint yy,FXint,FXint hh) const {
    register FXIcon *icon=(state&OPENED)?openIcon:closedIcon;
    register FXFont *font=list->getFont();
    register FXint th=0,tw=0,ih=0,iw=0;
    xx+=SIDE_SPACING/2;
    if(icon){
        iw=icon->getWidth();
        ih=icon->getHeight();
        dc.drawIcon(icon,xx,yy+(hh-ih)/2);
        xx+=ICON_SPACING+iw;
    }
    if (_extra_icon){
        iw=_extra_icon->getWidth();
        ih=_extra_icon->getHeight();
        dc.drawIcon(_extra_icon,xx,yy+(hh-ih)/2);
        xx+=ICON_SPACING+iw;
    }
    if(!label.empty()){
        tw=4+font->getTextWidth(label.text(),label.length());
        th=4+font->getFontHeight();
        yy+=(hh-th)/2;
        if(isSelected()){
            dc.setForeground(list->getSelBackColor());
            dc.fillRectangle(xx,yy,tw,th);
        }
        if(hasFocus()){
            dc.drawFocusRectangle(xx+1,yy+1,tw-2,th-2);
        }
        if(!isEnabled())
            dc.setForeground(makeShadowColor(list->getBackColor()));
        else if(isSelected())
            dc.setForeground(list->getSelTextColor());
        else
            dc.setForeground(_color);
        dc.drawText(xx+2,yy+font->getFontAscent()+2,label.text(),label.length());
    }

#if 0
    if (_extra_icon){
        if (!label.empty())
            xx += font->getTextWidth(label.text());
        
        iw=_extra_icon->getWidth();
        ih=_extra_icon->getHeight();
        dc.drawIcon(_extra_icon,xx,yy+(hh-ih)/2);
        xx+=ICON_SPACING+iw;
    }
#endif
    
}

} // ns component
} // ns gui
