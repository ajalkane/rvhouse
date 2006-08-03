#ifndef _GUI_COMPONENT_HOUSE_TREE_ITEM_H_
#define _GUI_COMPONENT_HOUSE_TREE_ITEM_H_

#include <fx.h>
#include <FXText.h>

namespace gui {
namespace component {

class house_tree_item : public FXTreeItem {
    FXDECLARE(house_tree_item)
    FXColor _color;
    FXIcon *_extra_icon;
public:
    house_tree_item();
    
    inline void color(FXColor c) { _color = c; }
    inline void extra_icon(FXIcon *i) { _extra_icon = i; }
    
protected:
    // Overrides the base class version of draw()
    virtual void draw(const FXTreeList* list,FXDC& dc,FXint xx,FXint yy,FXint,FXint hh) const;  
};

} // ns component
} // ns gui

#endif //_GUI_COMPONENT_HOUSE_TREE_ITEM_H_
