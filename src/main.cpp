#include <exception>
#include <iostream>

#include <ace/ACE.h>
#include <ace/Logging_Strategy.h>
#include <ace/Process_Manager.h>
#include <ace/Get_Opt.h>

#include <stdio.h>

#include "main.h"

#include "app_functionality/base.h"
#include "app_functionality/house.h"
#include "app_functionality/langdiff.h"
#include "app_functionality/export_contact_dat.h"
#include "app_functionality/import_contact_dat.h"

#include "gui/house_app.h"
// #include "gui/house_app.h"
#include "messaging/messenger.h"
#include "model/house.h"
#include "model/self.h"
#include "executable/launcher.h"
#include "multi_feed/config.h"
#include "lang/manager.h"
#include "lang/mapper.h"

#include "config_file.h"
#include "os_util.h"
#include "util.h"
#include "file_util.h"
#include "icon_store.h"
#include "icon_store.h"
#include "accessor.h"
#include "win_registry.h"
#include "app_options.h"
#include "app_version.h"

accessor<gui::house_app *> app(NULL);
accessor<gui::window::house *> house_win(NULL);
accessor<config_file *>    conf(NULL);
accessor<config_file *>    pref(NULL);
accessor<config_file *>    user_conf(NULL);
accessor<messenger *>      net_messenger(NULL);
accessor<messenger *>      gui_messenger(NULL);
accessor<model::house *>   house_model(NULL);
accessor<model::house *>   house_model_prev(NULL);
accessor<model::self *>    self_model(NULL);
accessor<executable::launcher *> launcher_game(NULL);
accessor<executable::launcher *> launcher_rvgl(NULL);
accessor<executable::launcher *> launcher_file(NULL);
accessor<executable::launcher *> launcher_rvtm(NULL);
accessor<win_registry *>  game_registry(NULL);
accessor<icon_store *>    app_icons(NULL);
accessor<lang::manager *> lang_mngr(NULL);

lang::mapper lang_mapper;
app_options  app_opts;

namespace {
    ACE_Logging_Strategy *logging_strategy = NULL;

    enum {
        opt_langdiff = 0,
        opt_export_contact_dat,
        opt_import_contact_dat,
    };

    app_functionality::base *app_mode = NULL;
}

#define CONF_FILE "conf/rvhouse.ini"
#define USER_CONF_FILE "conf/user_conf.ini"

app_functionality::base *
_create_app_functionality(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        ACE_DEBUG((LM_DEBUG, "main: argv[%d]: '%s'\n", i, argv[i]));
    }

    // Parse command line options first
    // static const ACE_TCHAR options[] = ":l:";
    // Long options only

    char_array_copy arg_copy(argc, argv);

    ACE_Get_Opt cmd_opts(arg_copy.size(), arg_copy.array(), ":", 1, 1, ACE_Get_Opt::PERMUTE_ARGS); // options, 1, 1);
    cmd_opts.long_option("langdiff",
                         opt_langdiff,
                         ACE_Get_Opt::ARG_REQUIRED);
    cmd_opts.long_option("export_contact_dat",
                         opt_export_contact_dat,
                         ACE_Get_Opt::ARG_REQUIRED);
    cmd_opts.long_option("import_contact_dat",
                         opt_import_contact_dat,
                         ACE_Get_Opt::ARG_REQUIRED);

    int option;
    while ((option = cmd_opts ()) != EOF) {
        switch (option) {
        case opt_langdiff:
            return new app_functionality::langdiff(
                argc, argv, DEFAULT_LANGUAGE, cmd_opts.opt_arg()
            );
        case opt_export_contact_dat:
            return new app_functionality::export_contact_dat(
                argc, argv, cmd_opts.opt_arg()
            );
        case opt_import_contact_dat:
            return new app_functionality::import_contact_dat(
                argc, argv, cmd_opts.opt_arg()
            );
        case ':':
            throw "option missing";
        default:
            throw "invalid option";
        }
    }

    return NULL;
}

void init_app(int &argc, char **argv) {
    ACE_DEBUG((LM_DEBUG, "init_app\n"));

    app_mode = _create_app_functionality(argc, argv);

    if (!app_mode) app_mode = new app_functionality::house(argc, argv);
}

void init_pre() {
    // First init configuration
    std::string conf_file      = app_rel_path(CONF_FILE);
    std::string user_conf_file = app_rel_path(USER_CONF_FILE);
    conf.instance(new config_file(config_file::ini_file));
    conf()->load(conf_file);
    user_conf.instance(new config_file(config_file::ini_file));
    user_conf()->load_conditionally(user_conf_file);

    // Init logging
    std::string logfile = conf()->get<std::string>("log/file", "stderr");
    std::string logsize = conf()->get<std::string>("log/size", "2000"); // 2MBytes
    std::string logamnt = conf()->get<std::string>("log/amount", "2"); // 2 log files
    std::string logintr = conf()->get<std::string>("log/interval", "10"); // interval in seconds for sampling log file size
    bool logdebug       = conf()->get<bool>       ("log/debug", true);

    if (logfile == "stderr") logfile.clear();
    if (!logfile.empty()) logfile = app_rel_path(logfile);
    std::string logpars = "VERBOSE_LITE";

    // logging strategy's init unfortunately
    // writes temporarily to the passed strings due
    // to using strtok... create a copy of the array for parsing

    const char *log_s_pars[] = {
        "-f", logpars.c_str(),
        "-m", logsize.c_str(),
        "-N", logamnt.c_str(),
        "-i", logintr.c_str(),
        "-s", logfile.c_str()
    };

    char_array_copy log_pars(array_sizeof(log_s_pars), log_s_pars);
    size_t log_s_pars_size = log_pars.size();

    if (logfile.empty()) log_s_pars_size = 1;

    logging_strategy = new ACE_Logging_Strategy;

    if (logging_strategy->init(log_s_pars_size,
                               log_pars.array()))
    {
        std::cerr << "Could not open log system, params used: " << std::endl;

        for (int i = 0; i < log_pars.size(); i++)
            std::cerr << log_pars.array()[i] << " ";
        std::cerr << std::endl;
    }

    ACE_DEBUG((LM_INFO, APP_NAME " " APP_VERSION " started\n"));
    if (!logdebug) {
        ACE_DEBUG((LM_DEBUG, "Turning debug logging off\n"));

        ACE_LOG_MSG->priority_mask(
            LM_NOTICE  | LM_ALERT | LM_INFO |
            LM_WARNING | LM_ERROR | LM_CRITICAL |
            LM_EMERGENCY,
            ACE_Log_Msg::PROCESS
        );
    }

    ACE_Process_Manager *pm = ACE_Process_Manager::instance();
    pm->open(10, ACE_Reactor::instance());
}

void init_debug() {
    if (app_opts.debug()) {
        multi_feed::config::always_display_feeds = true;
    }
}

int do_main(int &argc, char **argv) {
    // ACE_LOG_MSG->set_flags(ACE_Log_Msg::VERBOSE_LITE);
    ACE_LOG_MSG->set_flags(ACE_Log_Msg::VERBOSE);

    init_pre();
    init_debug();
    init_app(argc, argv);

    ACE_DEBUG((LM_DEBUG, "run app %d\n", app_mode));
    app_mode->run();

    // pref()->save();

    return 0;
}

int
ACE_TMAIN (int argc, ACE_TCHAR *argv[]) {
    try {
        ACE_DEBUG((LM_DEBUG, "starting main at thread %t\n"));
        do_main(argc, argv);
    } catch (const std::exception &e) {
        ACE_DEBUG((LM_DEBUG, "exception received at thread %t:\n"));
        ACE_DEBUG((LM_DEBUG, "exception received: %s\n", e.what()));
        os::alert("Fatal error received", e.what());
//    } catch (const FXException &e) {
//        ACE_DEBUG((LM_DEBUG, "exception received2  at thread %t:\n"));
//        os::alert("Fatal error received", e.what());
    } catch (const char *e) {
        ACE_DEBUG((LM_DEBUG, "exception received3  at thread %t:\n"));
        os::alert("Fatal error received", e);
    } catch (...) {
        ACE_DEBUG((LM_DEBUG, "exception received4  at thread %t:\n"));
        os::alert("Unrecognized exception", "Die...");
    }

    delete app_mode;
//    delete app();
    delete app();
    ACE_DEBUG((LM_DEBUG, "Deleting conf\n"));
    delete conf();
    delete user_conf();
    // delete pref();

    // ACE_DEBUG((LM_DEBUG, "Deleting logging_strategy\n"));
    // Seems maybe logging strategy is not meant to be deleted manually,
    // documentation on this issue in ACE is scarce.
    // delete logging_strategy;
    ACE_DEBUG((LM_INFO, APP_NAME " exit\n"));
    return 0;
}
