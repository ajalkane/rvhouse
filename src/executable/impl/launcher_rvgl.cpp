#include <ace/Process_Manager.h>
#include <ace/Reactor.h>

#include <vector>
#include <string>

#include "../../main.h"
#include "../../config_file.h"
#include "../../regexp.h"
#include "../../app_version.h"
#include "../../win_registry.h"
#include "../../messaging/message.h"
#include "../../messaging/messenger.h"
#include "launcher_rvgl.h"

namespace executable {

launcher_rvgl::launcher_rvgl() 
    : _rvgl_pid(ACE_INVALID_PID),
      _running(false) 
{  
}

launcher_rvgl::~launcher_rvgl() {
    if (_rvgl_pid != ACE_INVALID_PID) {
        ACE_DEBUG((LM_INFO, "launcher_rvgl::dtor removing pid %d\n",
                   _rvgl_pid));
        ACE_Process_Manager *pm = ACE_Process_Manager::instance();
        pm->remove(_rvgl_pid);
    }
}

int 
launcher_rvgl::start_host() {
    return _launch("");
}

int
launcher_rvgl::start_client(const std::string &host_id) {
    return _launch(host_id);
}

int
launcher_rvgl::_launch(const std::string &host_id) {
    if (_running) return err_already_running;

    std::string dir = pref()->get<std::string>("advanced/rvgl_path", "");
    std::string params = pref()->get<std::string>("advanced/rvgl_cmdline", "");

    if (dir.empty()) {
        win_registry r(win_registry::id_dplay, "", "Re-Volt");
        dir = r.get<std::string>("Path", "");
        pref()->set("advanced/rvgl_path", dir.c_str());
    }

    std::string cmd(dir);
#ifdef WIN32
    cmd += "\\rvgl.exe";
#else
    cmd += "/rvgl";
#endif
    cmd = "\"" + cmd + "\"";
    if (!params.empty()) cmd += " " + params;
    cmd += (host_id.empty() ? " -lobby" : " -lobby " + host_id);

    //printf("%s\n", cmd.c_str());
    ACE_DEBUG((LM_DEBUG, "launcher_rvgl: command line: %s\n",
              cmd.c_str()));

    // Launch options
    ACE_Process_Manager *pm = ACE_Process_Manager::instance();
    ACE_Process_Options opts;
    opts.working_directory(dir.c_str());
    opts.command_line(cmd.c_str());
    _rvgl_pid = pm->spawn(opts, this);
    ACE_DEBUG((LM_INFO, "launcher_rvgl: pid %d from thread %t, cmd: %s\n",
               _rvgl_pid, cmd.c_str()));
    if (_rvgl_pid == ACE_INVALID_PID) {
        ACE_ERROR((LM_ERROR, "launcher_rvgl: failed to launch: %s\n",
                  cmd.c_str()));
        return err_could_not_launch_rvgl;
    }
    
    _running = true;
    
    return 0;
}

int 
launcher_rvgl::handle_exit(ACE_Process *proc) {
    ACE_DEBUG((LM_INFO, "launcher_rvgl::handle_exit: called from thread %t\n"));
    // There's a potential risk here since handle_exit is called from 
    // different thread than _launch. But since integer operations are
    // usually atomic and the risk is at most theoretical, no locking done.
    _running = false;
    _rvgl_pid = ACE_INVALID_PID;
    return 0;   
}

} // ns executable


