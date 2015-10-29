#ifndef MAIN_H_
#define MAIN_H_

#include "accessor.h"

// Forward declarations
namespace model {
    class house;
    class self;
};

namespace executable {
    class launcher;
}

namespace lang {
    class mapper;
    class manager;
}

namespace gui {
    class house_app;

    namespace window {
        class house;
    }
}

extern accessor<class gui::house_app *>    app;
extern accessor<class gui::window::house *>    house_win;
extern accessor<class config_file *>  conf;
extern accessor<class config_file *>  user_conf;
extern accessor<class config_file *>  pref;
extern accessor<class messenger *>    net_messenger;
extern accessor<class messenger *>    gui_messenger;
extern accessor<class model::house *> house_model;
extern accessor<class model::house *> house_model_prev;
extern accessor<class model::self  *> self_model;
extern accessor<class executable::launcher *> launcher_game;
extern accessor<class executable::launcher *> launcher_rvgl;
extern accessor<class executable::launcher *> launcher_file;
extern accessor<class executable::launcher *> launcher_rvtm;
extern accessor<class win_registry *>  game_registry;
extern accessor<class icon_store *>    app_icons;
extern accessor<class icon_store *> app_icons;
extern accessor<class lang::manager *> lang_mngr;

extern class lang::mapper lang_mapper;
extern class app_options app_opts;

#endif //MAIN_H_
