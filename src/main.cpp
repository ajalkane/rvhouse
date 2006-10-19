#include <exception>
#include <iostream>

#include <ace/ACE.h>
#include <ace/Logging_Strategy.h>
#include <ace/Process_Manager.h>
#include <ace/Get_Opt.h>
#include <fx.h>

#include <stdio.h>

#include "app_functionality/base.h"
#include "app_functionality/house.h"
#include "app_functionality/langdiff.h"

#include "gui/house_app.h"
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
#include "accessor.h"
#include "win_registry.h"
#include "app_options.h"
#include "app_version.h"

accessor<gui::house_app *> app(NULL);
accessor<config_file *>    conf(NULL);
accessor<config_file *>    pref(NULL);
accessor<messenger *>      net_messenger(NULL);
accessor<messenger *>      gui_messenger(NULL);
accessor<model::house *>   house_model(NULL);
accessor<model::house *>   house_model_prev(NULL);
accessor<model::self *>    self_model(NULL);
accessor<executable::launcher *> launcher_game(NULL);
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
    };
    
    app_functionality::base *app_mode = NULL;
}

#define CONF_FILE "conf/rvhouse.ini"

void init_app(int argc, char **argv) {
    ACE_DEBUG((LM_DEBUG, "init_app\n"));
    // Parse command line options first
    // static const ACE_TCHAR options[] = ":l:";
    // Long options only
    ACE_Get_Opt cmd_opts(argc, argv, ":", 1, 1, ACE_Get_Opt::PERMUTE_ARGS, 1); // options, 1, 1);
    cmd_opts.long_option("langdiff", opt_langdiff, ACE_Get_Opt::ARG_REQUIRED);
    
    int option;
    while ((option = cmd_opts ()) != EOF) {
        switch (option) {
        case opt_langdiff:
            app_mode = new app_functionality::langdiff(
                argc, argv, DEFAULT_LANGUAGE, cmd_opts.opt_arg()
            );
            break;
        case ':':
            throw "option missing";
            break;          
        default:
            throw "invalid option";
        }
    }
    
    if (!app_mode) app_mode = new app_functionality::house(argc, argv); 
}

void init_pre() {
    // First init configuration
    std::string conf_file = app_rel_path(CONF_FILE);                  
    conf.instance(new config_file());
    conf()->load(conf_file);
    // Preferences are settings that user has made,
    // and therefore have to persist across new installations
    // in contrast to config
/*    pref.instance(
        new config_file(
            conf()->get<std::string>("user", "app_id",    "RV House"),
            conf()->get<std::string>("user", "vendor_id", "Re-Volt")
        )
    );
    
    pref()->load();
    
    app_opts.init();*/
    
    // Init logging
    std::string logfile = conf()->get<std::string>("log", "file", "log/rvhouse.log");
    std::string logsize = conf()->get<std::string>("log", "size", "2000"); // 2MBytes
    std::string logamnt = conf()->get<std::string>("log", "amount", "2"); // 2 log files
    std::string logintr = conf()->get<std::string>("log", "interval", "10"); // 10 interval for sampling log file size
    bool logdebug       = conf()->get<bool>       ("log", "debug", true);
    
    logfile = app_rel_path(logfile);
    std::string logpars = "VERBOSE_LITE";
    
    const char *log_s_pars[] = {
        "-s", logfile.c_str(),
        "-m", logsize.c_str(),
        "-N", logamnt.c_str(),
        "-i", logintr.c_str(),
        "-f", logpars.c_str()
    };

    logging_strategy = new ACE_Logging_Strategy;
    
    // TODO logging strategy's init unfortunately
    // writes temporarily to the passed strings due
    // to using strtok... using std::strings 
    // as values is technically wrong but will do for
    // now. Better to make a copy of the log_s_pars,
    // maybe with an encapsulating class    
    if (logging_strategy->init(array_sizeof(log_s_pars), 
                               const_cast<char **>(log_s_pars))) 
    {
        std::cerr << "Could not open log system, params used: " << std::endl;
                               
        for (unsigned int i = 0; i < array_sizeof(log_s_pars); i++)
            std::cerr << log_s_pars[i] << " ";
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

int do_main(int argc, char **argv) {
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
    } catch (const FXException &e) {
        ACE_DEBUG((LM_DEBUG, "exception received2  at thread %t:\n"));
        os::alert("Fatal error received", e.what());
    } catch (const char *e) {
        ACE_DEBUG((LM_DEBUG, "exception received3  at thread %t:\n"));
        os::alert("Fatal error received", e);       
    } catch (...) {
        ACE_DEBUG((LM_DEBUG, "exception received4  at thread %t:\n"));
        os::alert("Unrecognized exception", "Die...");
    }

    delete app_mode;
    delete app();
    ACE_DEBUG((LM_DEBUG, "Deleting conf\n"));
    delete conf();
    // delete pref();
    
    // ACE_DEBUG((LM_DEBUG, "Deleting logging_strategy\n"));
    // Seems maybe logging strategy is not meant to be deleted manually,
    // documentation on this issue in ACE is scarce.
    // delete logging_strategy;
    ACE_DEBUG((LM_INFO, APP_NAME " exit\n"));   
    return 0;
}
