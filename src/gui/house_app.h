#ifndef HOUSE_APP_H_
#define HOUSE_APP_H_

#include <QApplication>

#include <string>
#include <list>
#include <map>

#include "../chat_gaming/user.h"
#include "../chat_gaming/room.h"
#include "../messaging/message.h"
#include "../model/house.h"
#include "../networking/worker.h"

#include "window/house.h"
#include "window/room.h"
#include "window/private_message.h"

namespace gui {
    
class house_app : public QApplication {
    // Q_OBJECT

    // Begin of filter structures
    // Duplicates filtering structures
    struct _filter_dup_struct {
        const static size_t max_seqs = 4;
        std::list<unsigned> seqs;
    };
    typedef std::map<
        // Key is user id_str
        std::string,
        _filter_dup_struct
    > _filter_dup_map_type;

    _filter_dup_map_type _filter_dup_map;

    networking::worker *_net_worker;

    typedef std::list<QObject *> _watched_list_type;
    _watched_list_type           _watched_list;

    window::house *_house_win;
    window::room     *_room_win;

    void _init_language();
    void _handle_model_update(model::house *hm, ::message *msg);
    void _handle_user_model_update(model::house *hm, ::message *msg);
    void _handle_room_model_update(model::house *hm, ::message *msg);
    void _handle_room_join(::message *msg);
    void _handle_room_join_ack(::message *msg);
    void _handle_global_ignore(::message *msg);
    void _handle_worker_exception(message *om);
    void _cond_open_room_window(::message *msg);
    void _cond_open_private_message_window(::message *msg);
    void _version_update_window(::message *msg);

    bool _handle_messenger();
    bool _filter_message(::message *msg);
    bool _filter_dup(const std::string &sndr_id, unsigned seq,
                     _filter_dup_map_type *map,
                     int group_base);



public:
    house_app(int &argc, char **argv);
    virtual ~house_app();

    bool start();

    void interruptable_action_update();
    void interruptable_action_update(const std::string &s);

    inline void status_message(const std::string &msg) {
        _house_win->status_message(msg);
    }
    class window::private_message *private_message_window_to(
        const std::string &channel,
        const std::string &user_id_str,
        bool raise,
        bool allow_new_window   = true,
        bool *new_window_needed = NULL
    );

    void watched_object_created(QObject *obj);
    void watched_object_destroyed(QObject *obj);

    // Qt override for handling events from networking thread
    bool event(QEvent *event);
};

} // ns gui

#endif //HOUSE_APP_H_
