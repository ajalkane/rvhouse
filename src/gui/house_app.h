#ifndef _HOUSE_APP_H_
#define _HOUSE_APP_H_

#include <fx.h>

#include <string>
#include <list>
#include <map>

#include "../common.h"
#include "../messaging/messenger.h"
#include "../networking/worker.h"
#include "../config_file.h"
#include "../chat_gaming/user.h"
#include "../chat_gaming/room.h"
#include "../model/house.h"
#include "window/house.h"
#include "window/login.h"
#include "window/room.h"
#include "window/private_message.h"

namespace gui {

class house_app : public FXApp {
    FXDECLARE(house_app)

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

#if 0
    // Removed processing of old sequences because if this is
    // wanted must implement some things:
    // - each group must receive model updates that are newer than
    //   received for the group previously
    // TODO... requires some pondering... not too important for
    // now, to be done when time permits.
    struct _filter_old_struct {
        const static size_t seq_window_size = 10;
        unsigned newest_seq;
    };

    typedef std::map<
    // Key is user id_str.
    std::string,
    // Value is last sequence seen.
    _filter_old_struct
    > _filter_old_map_type;

    typedef std::map<
    // Key is user id_str.
    std::string,
    // Value is last sequence seen.
    _filter_old_struct
    > _filter_pre_old_map_type;

    _filter_old_map_type _filter_user_map;
    _filter_old_map_type _filter_room_map;
#endif
    // End of filter structures

    // typedef std::list<FXWindow *> _window_list_type;
    typedef std::list<FXObject *> _watched_list_type;
    _watched_list_type            _watched_list;

    window::house *_house_win;
    FXWindow      *_room_win;

    std::string         _app_dir;
    networking::worker *_net_worker;
    // chat_gaming::user   _user; // Self prototype
    // chat_gaming::room   _room; // Host room prototype

    void _init_language();
    void _handle_model_update(model::house *hm, ::message *msg);
    void _handle_user_model_update(model::house *hm, ::message *msg);
    void _handle_room_model_update(model::house *hm, ::message *msg);
    void _handle_room_join(::message *msg);
    void _handle_room_join_ack(::message *msg);
    void _cond_open_room_window(::message *msg);
    void _cond_open_private_message_window(::message *msg);
    bool _filter_message(::message *msg);
    bool _filter_dup(const std::string &sndr_id, unsigned seq,
                     _filter_dup_map_type *map,
                     int group_base);

    void _launch_rvtm_error(int err);
#if 0
    bool _filter_old(const std::string &sndr_id, unsigned seq,
                     _filter_old_map_type *map,
                     int group_base);
#endif
public:
    enum {
        ID_MESSENGER = FXApp::ID_LAST,
        ID_TEST,
        // These for watched objects
        ID_OBJECT_CREATE,
        ID_OBJECT_DESTROY,
        ID_LAST,
    };

    house_app();
    ~house_app();

    virtual void create();

    // Call this instead of run when starting the application
    void start();

    void interruptable_action_update();
    void interruptable_action_update(const std::string &s);

    void version_update_window(::message *msg);
    void launch_rvtm();
    void launch_rvtm(const std::string &host_ip);

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

    long handle_messenger(FXObject *from, FXSelector sel, void *msnger_p);
    void handle_worker_exception(message *om);

    long on_object_create (FXObject *from, FXSelector sel, void *ptr);
    long on_object_destroy(FXObject *from, FXSelector sel, void *ptr);

    //	long on_window_create (FXObject *from, FXSelector sel, void *ptr);
    //	long on_window_destroy(FXObject *from, FXSelector sel, void *ptr);
    long on_test(FXObject *from, FXSelector sel, void *ptr);
};

} // ns gui

#endif //_HOUSE_APP_H_
