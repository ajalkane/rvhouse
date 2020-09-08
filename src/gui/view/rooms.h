#ifndef ROOMS_VIEW_H
#define ROOMS_VIEW_H

#include <QAction>
#include <QActionGroup>
#include <QList>
#include <QEvent>
#include <QMenu>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <map>

#include "users.h"

#include "../../common.h"
#include "../../messaging/message.h"
#include "../../chat_gaming/user.h"
#include "../../chat_gaming/room.h"
#include "../../multi_feed/room_item.h"
#include "../../multi_feed/room_map.h"

namespace gui {
namespace view {

class room_item : public QTreeWidgetItem {
    typedef QTreeWidgetItem super;
public:
    typedef chat_gaming::room room_type;
private:
    multi_feed::room_item      _room_feed;
    chat_gaming::room::id_type _room_id;
    
    QString  _tip_str;
    
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

    inline chat_gaming::room::id_type room_id() {
        return _room_id;
    }

    // Returns true if the room should be removed for good from list
    bool room_remove(const chat_gaming::room::id_type &id, int grp);
    
    void resolve_room_state();
    bool update_tip();
    inline const QString &get_tip() const { return _tip_str; }

};

class rooms : public QTreeWidget, public users::observer {
    Q_OBJECT

public:
    enum columns {
        column_topic = 0,
        column_host,
        column_players,
        column_laps,
        column_pickups,
        column_rvgl,

        column_count
    };

    typedef QTreeWidget super;
    typedef QTreeWidgetItem tree_item_type;
    typedef room_item  item_type;
    
    class observer {
    public:
        virtual void room_added(const chat_gaming::room &r)   {}
        virtual void room_removed(const chat_gaming::room &u) {}
    };
    inline void observer_set(observer *o) { _observer = o; }
    
    rooms(QWidget *parent);
    
    void handle_message(::message *msg);

    inline item_type *selected_item() const {
        QTreeWidgetItem *item = this->currentItem();
        return static_cast<item_type *>(item);
    }

signals:
    void join_selected_room_requested();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:    
    observer *_observer;

    // Maps a room to the list item displaying it.
    multi_feed::room_map<item_type> _room_item_map;

    void _create_header();

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

#endif //ROOMS_VIEW_H
