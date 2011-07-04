#ifndef USERS_VIEW_H_
#define USERS_VIEW_H_

#include <QAction>
#include <QActionGroup>
#include <QEvent>
#include <QMenu>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>

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
// #include "../component/house_tree_item.h"

namespace gui {

// Have to make a forward declaration to avoid cyclical dependency
namespace window {
    class house;
}

namespace view {

class user_item : public QTreeWidgetItem {
    typedef QTreeWidgetItem super;
public:
    typedef chat_gaming::user user_type;
    
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
    enum {
        column_name = 0
    };

    bool _dht_conn;
    bool _ctz_conn;
    bool _validated;
    bool _natted;
    bool _sharing_tracks;
    bool _getting_tracks;
        
    int                     _display_group;
    multi_feed::user_item   _user_feed;

    QString _tip_str;
    
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
    }

    inline const multi_feed::user_item &feed_item() const { return _user_feed; }
    
    inline int display_group() const { return _display_group; }
    
    // Returns true if the user should be removed for good from list
    // bool user_remove(const chat_gaming::user *u);
    bool user_remove(const chat_gaming::user::id_type &id, int grp);
    
    // Returns true if the (visible) user state has changed in a way
    // that needs redrawing
    bool resolve_user_state();
    bool update_tip();
    inline const QString &get_tip() const { return _tip_str; }
    inline bool sharing_tracks() const { return _sharing_tracks; }
    inline bool getting_tracks() const { return _getting_tracks; }
};

class users : public QTreeWidget {
    Q_OBJECT

public:
    typedef QTreeWidget super;
    typedef QTreeWidgetItem tree_item_type;
    typedef user_item  item_type;

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
                                     QEvent *e) {}
        
    };
    
    users(QWidget *parent, const chat_gaming::room::id_type &rid = std::string());
    virtual ~users();

    inline item_type *find_item(const chat_gaming::room::id_type &id) {
        return _user_item_map.find(id);
    }

    inline item_type *selected_item() const {
        QTreeWidgetItem *item = this->currentItem();
        return static_cast<item_type *>(item);
    }

    void handle_message(::message *msg);

    inline void observer_set(observer *o) { _observer = o; }
protected:
    bool eventFilter(QObject *obj, QEvent *event);
private:
    QMenu *_context_menu;
    QAction *_action_user;

    QMenu *_status_submenu;
    QActionGroup *_action_statusgroup;
    QAction *_action_chatting;
    QAction *_action_playing;
    QAction *_action_away;
    QAction *_action_donotdisturb;
    QAction *_action_block;
    QAction *_action_private_message;
    QAction *_action_kick;

    observer *_observer;
    
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
    
    multi_feed::user_map<item_type> _user_item_map;
    
    void _create_actions();
    void _create_context_menu();
    void _create_signals();

    void _add_user_item(item_type *, int dgrp = -1);
    void _add_user_to_group(tree_item_type *parent, item_type *item);

    void _del_user_item(item_type *,  int dgrp = -1);
    tree_item_type *_reserve_place_for_item_in_group(int dgrp);
    void _remove_place_for_item_in_group(int dgrp);

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

public slots:
    void on_context_menu(const QPoint &pos);
    void block_selected_user();
    void kick_selected_user();
    void open_private_msg_to_selected_user();
    void toggle_selection(QTreeWidgetItem *item, int column);
};

} // ns view
} // ns gui

#endif //USERS_VIEW_H_
