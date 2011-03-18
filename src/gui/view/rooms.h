#ifndef _ROOMS_VIEW_H_
#define _ROOMS_VIEW_H_

#include <fx.h>
#include <FXText.h>

#include <map>

#include "../../common.h"
#include "../../messaging/message.h"
#include "../../chat_gaming/user.h"
#include "../../chat_gaming/room.h"
#include "../../multi_feed/room_item.h"
#include "../../multi_feed/room_map.h"

namespace gui {
namespace view {

class room_item : public FXIconItem {
    FXDECLARE(room_item)
public:
    typedef chat_gaming::room room_type;
private:
    multi_feed::room_item      _room_feed;
    chat_gaming::room::id_type _room_id;
    
    FXString  _tip_str;
    
    bool _resolve_columns();
    bool _resolve_tip();
    std::string _tip_participants();
    inline std::string _tip(const std::string &s) {
        return (s.empty() 
                ? std::string() 
                : s + '\n');
    }
    
public:
    room_item(const chat_gaming::room::id_type &rid);

    inline const multi_feed::room_item &feed_item() const { return _room_feed; }
    inline void room(const chat_gaming::room::id_type &id, int grp_base) {
        _room_feed.set_feed(id, grp_base);
    }

    // Returns true if the room should be removed for good from list
    bool room_remove(const chat_gaming::room::id_type &id, int grp);
    
    // Returns true if the (visible) room state has changed in a way
    // that needs redrawing
    bool resolve_room_state();

    
    long on_query_tip(FXObject *sender,FXSelector sel,void *ptr);
protected:
    room_item();
};

class rooms : public FXIconList, public users::observer {
    FXDECLARE(rooms)

    struct _handler_target_type {
        FXObject   *target;
        FXSelector  sel;
        void       *ptr;
        inline _handler_target_type() : target(NULL),sel(0),ptr(NULL) {}
    };
      
    _handler_target_type _handler_item_doubleclicked;

public:
    typedef FXIconList super;
    typedef FXIconItem tree_item_type;
    typedef room_item  item_type;
    
    class observer {
    public:
        virtual void room_added(const chat_gaming::room &r)   {}
        virtual void room_removed(const chat_gaming::room &u) {}
    };
    inline void observer_set(observer *o) { _observer = o; }
    
    rooms(FXComposite *c, FXObject *tgt=NULL, 
               FXSelector sel=0, 
               FXuint opts=LAYOUT_FILL_X|LAYOUT_FILL_Y|
                           ICONLIST_DETAILED|ICONLIST_EXTENDEDSELECT|
                           LAYOUT_FILL_X|
                           LAYOUT_FILL_Y,
               FXint x=0, FXint y=0, FXint w=0, FXint h=0);
    virtual void create();
    
    void handle_message(::message *msg);    

    inline void target_item_doubleclicked(FXObject *t, FXSelector sel) {
        _handler_item_doubleclicked.target = t;
        _handler_item_doubleclicked.sel    = sel;
        _handler_item_doubleclicked.ptr    = NULL;
    }
    
    long on_query_tip       (FXObject *sender,FXSelector sel,void *ptr);
    long on_room_doubleclick(FXObject *sender,FXSelector sel,void *ptr);
    
    int selected_item_index() const;

    inline const room_item *item_at(int ndx) const { 
        return dynamic_cast<const room_item *>(getItem(ndx));
    }

    inline chat_gaming::room::id_type room_id_at(int ndx) const {
        const room_item *item = item_at(ndx);
        if (!item) return chat_gaming::room::id_type();
        return _room_item_map.find_key(const_cast<room_item *>(item));
    }
    
    inline void removeItem(FXIconItem *i, FXbool notify = FALSE) {
        item_to_index_call<void>(&super::removeItem, notify, i);
    }
    inline void updateItem(FXIconItem *i) {
        item_to_index_call<void>(&super::updateItem, i);
    }
    inline void enableItem(FXIconItem *i) {
        item_to_index_call<FXbool>(&super::enableItem, i);
    }
    inline void disableItem(FXIconItem *i) {
        item_to_index_call<FXbool>(&super::disableItem, i);
    }

    template <class RetVal>
    void
    item_to_index_call(RetVal (super::*method)(FXint), FXIconItem *item) {
        int i = super::findItemByData(item);
        ACE_DEBUG((LM_DEBUG, "rooms::item_to_index_call item search index %d\n", i));
        if (i >= 0) {
            (this->*method)(i);
        }
    }
    
    template <class RetVal>
    void
    item_to_index_call(RetVal (super::*method)(FXint,FXbool), FXbool v, FXIconItem *item) {
        int i = super::findItemByData(item);
        ACE_DEBUG((LM_DEBUG, "rooms::item_to_index_call item search index %d\n", i));
        if (i >= 0) {
            (this->*method)(i, v);
        }
    }
    
    template <class RetVal>
    void 
    item_to_index_call(RetVal (super::*method)(FXint) const, FXIconItem *item) {
        int i = super::findItemByData(item);
        ACE_DEBUG((LM_DEBUG, "rooms::item_to_index_call item search index %d\n", i));
        if (i >= 0) {
            (this->*method)(i);
        }
    }
    
protected:
    rooms() {}
private:    
    observer *_observer;

    // Maps a room to the list item displaying it.
    multi_feed::room_map<item_type> _room_item_map;

    void _handle_user_update(::message *msg);   
    
    void _update_room(const chat_gaming::room &r, int grp_base);
    void _update_room(const chat_gaming::room::id_type &rid, int grp_base);
    void _update_room_state(item_type *item);
    
    void _remove_room(const chat_gaming::room &r, int grp_base);
    void _new_room   (const chat_gaming::room &r, int grp_base);
    void _remove_dropped();
};

} // ns view
} // ns gui

#endif //_ROOMS_VIEW_H_
