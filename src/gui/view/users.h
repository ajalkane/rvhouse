#ifndef _USERS_VIEW_H_
#define _USERS_VIEW_H_

#include <fx.h>
#include <FXText.h>

#include <map>

#include "../../util.h"
#include "../../common.h"
#include "../../exception.h"
#include "../../messaging/message.h"
#include "../../chat_gaming/user.h"
#include "../../chat_gaming/room.h"
#include "../../model/house.h"
#include "../../multi_feed/user_item.h"
#include "../../multi_feed/user_map.h"
#include "../component/house_tree_item.h"

namespace gui {
namespace view {

class user_item : public component::house_tree_item {
    FXDECLARE(user_item)
public:
    typedef chat_gaming::user user_type;
    // Maps users source group -> user id
    // typedef std::map<int, chat_gaming::user::id_type> user_map_type;
    // typedef std::map<int, std::string>                group_desc_map_type;   
    
    enum {
        // Have to be in the same order as they'll be 
        // shown in the view
        group_chatting = 0,
        group_in_room,
        group_playing,
        group_away,
        group_dnd, // Do not disturb group
        group_size,
    };
private:
    const static FXColor col_dht_conn;
    const static FXColor col_ctz_conn;
    const static FXColor col_dht_ctz_conn;
    const static FXColor col_nat_dht_ctz_conn;
    const static FXColor col_not_validated;

    bool _dht_conn;
    bool _ctz_conn;
    bool _validated;
    bool _natted;
    bool _sharing_tracks;
    bool _getting_tracks;
        
    int                     _display_group;
    multi_feed::user_item   _user_feed;

    FXString _tip_str;
    
    bool _resolve_user_id();
    bool _resolve_tip();

    inline std::string _tip(const std::string &s) {
        return (s.empty() 
                ? std::string() 
                : s + '\n');        
    }
public:
    user_item();
        
    // Getter and setter for user
    inline void user(const chat_gaming::user::id_type &id, int grp_base) {
        _user_feed.set_feed(id, grp_base);
    //  _user_map[grp_base] = id;
    }

    inline const multi_feed::user_item &feed_item() const { return _user_feed; }
    
    inline int display_group() const { return _display_group; }
    
    // Returns true if the user should be removed for good from list
    // bool user_remove(const chat_gaming::user *u);
    bool user_remove(const chat_gaming::user::id_type &id, int grp);
    
    // Returns true if the (visible) user state has changed in a way
    // that needs redrawing
    bool resolve_user_state();

    inline bool sharing_tracks() const { return _sharing_tracks; }
    inline bool getting_tracks() const { return _getting_tracks; }
    
    long on_query_tip(FXObject *sender,FXSelector sel,void *ptr);
protected:
    // user_item() {}
};

class users : public FXTreeList {
    FXDECLARE(users)
public:
    typedef FXTreeList super;
    typedef FXTreeItem tree_item_type;
    typedef user_item  item_type;

    enum {
        ID_STATUS_CHATTING = super::ID_LAST,
        ID_STATUS_PLAYING,
        ID_STATUS_AWAY,
        ID_STATUS_DONT_DISTURB,
        
        ID_USER_KICK,
        ID_USER_BLOCK,
        ID_SHARE_TRACKS,
        ID_GET_TRACKS,
        
        ID_PRIV_MSG,
        
        ID_LAST,
    };
    
    class observer {
    public:
        virtual void user_added(const chat_gaming::user &u)   {}
        virtual void user_removed(const chat_gaming::user &u) {}
        virtual void user_blocked(const std::string &display_id) {}
        
        virtual void sharing_tracks_changed(const chat_gaming::user &u,
                                            bool value) {}
        virtual void getting_tracks_changed(const chat_gaming::user &u,
                                            bool value) {}
        
        virtual void user_rightclick(const multi_feed::user_item &uf,
                                     FXEvent *e) {}
        
    };
    
    users(FXComposite *c, FXObject *tgt=NULL, 
              FXSelector sel=0, 
              FXuint opts=TREELIST_SINGLESELECT|
                          TREELIST_SHOWS_BOXES |
                          TREELIST_ROOT_BOXES  |
                          LAYOUT_FILL_X|
                          LAYOUT_FILL_Y,
              FXint x=0, FXint y=0, FXint w=0, FXint h=0);
    virtual void create();
    virtual ~users();

    inline void room_id(const chat_gaming::room::id_type &rid) {
        _room_id = rid;
    }
    
    inline item_type *find_item(const chat_gaming::room::id_type &id) {
        return _user_item_map.find(id);
    }
    // DUMMY implementation, TODO there must be a better way    
    inline item_type *selected_item() const {
        FXTreeItem *last = getLastItem();
        FXTreeItem *iter = getFirstItem();
        ACE_DEBUG((LM_DEBUG, "users::selected_item: first/last: %d/%d\n",
                  iter, last));
        for (; iter; iter = iter->getBelow()) {
            ACE_DEBUG((LM_DEBUG, "users::selected_item: iter: %s/%d\n",
                      iter->getText().text()));
            if (isItemSelected(iter)) return dynamic_cast<item_type *>(iter);
        }
        ACE_DEBUG((LM_DEBUG, "users::selected_item: not found\n",
                  iter, last));
        return NULL;
    }
    
    void handle_message(::message *msg);

    long on_query_tip(FXObject *sender,FXSelector sel,void *ptr);
    long on_user_rightclick(FXObject *sender,FXSelector sel,void *ptr);
    long on_status_change(FXObject *sender,FXSelector sel,void *ptr);
    long on_user_kick(FXObject *sender,FXSelector sel,void *ptr);
    long on_user_block(FXObject *sender,FXSelector sel,void *ptr);
    long on_share(FXObject *sender,FXSelector sel,void *ptr);
    long on_doubleclick(FXObject *sender,FXSelector sel,void *ptr);
    long on_priv_msg(FXObject *sender,FXSelector sel,void *ptr);
    
    inline void observer_set(observer *o) { _observer = o; }
protected:
    users() {}
private:
    FXMenuPane    *_popup;
    FXMenuPane    *_popup_status;
    FXMenuCascade *_popup_status_cascade;
    FXMenuCommand *_popup_user;
    FXMenuCommand *_popup_share;
    FXMenuCommand *_popup_priv;
    FXMenuCommand *_popup_block;
    
    observer *_observer;
    
    // tree_item_type *_item_chatting;
    // tree_item_type *_item_waiting;
    // tree_item_type *_item_playing;   
    // tree_item_type *_item_idling;
    struct _parent_item {
        tree_item_type *item;
        int             children;
        const char     *text;
        _parent_item(const char *txt = NULL) 
          : item(NULL), children(0), text(txt) {}
        inline bool visible() const { return children > 0; }
    };
    _parent_item _group_items[user_item::group_size];
    
    tree_item_type *_item_players; // only one category for now
    chat_gaming::room::id_type _room_id;    
    
    // Maps a user to the list item displaying it.
    // typedef std::map<chat_gaming::user::id_type, item_type *> _user_item_map_type;

    // typedef std::map<std::string, item_type *> _user_item_map_type;  
    //_user_item_map_type _user_item_map;
    
    multi_feed::user_map<item_type> _user_item_map;
    
    void _add_user_item(item_type *, int dgrp = -1);
    void _del_user_item(item_type *,  int dgrp = -1);
    tree_item_type *_add_user_item2(int dgrp);
    void _del_user_item2(int dgrp);

    void _update_user_item(item_type *,int old_dgrp);
    
    inline _parent_item &_get_group_item(int dgrp) {
        if (dgrp < 0 || dgrp >= (int)array_sizeof(_group_items)) {
            throw exceptionf(0, "users::_get_group_item unrecognized %d", dgrp);
        }
        return _group_items[dgrp];      
    }
    void _add_group_item(_parent_item &group_item, int dgrp);
    void _del_group_item(_parent_item &group_item);
    
    void _update_user(const chat_gaming::user::id_type &uid, int grp_base);
    void _update_user(const chat_gaming::user &u, int grp_base);
    void _remove_user(const chat_gaming::user &u, int grp_base);
    void _new_user   (const chat_gaming::user &u, int grp_base);
    void _remove_dropped(); 
    void _update_users(const chat_gaming::room::id_type &rid, int grp_base);
    void _room_update(const chat_gaming::room::id_type &rid, int grp_base);

    bool _priv_msg_enable(const item_type *item) const;
    
};

} // ns view
} // ns gui

#endif //_USERS_VIEW_H_
