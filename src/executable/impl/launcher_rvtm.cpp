#include <ace/Process_Manager.h>
#include <ace/Reactor.h>

#include <windows.h>

#include <vector>
#include <string>

#include "../../main.h"
#include "../../regexp.h"
#include "../../app_version.h"
#include "../../win_registry.h"
#include "../../messaging/message.h"
#include "../../messaging/messenger.h"
#include "launcher_rvtm.h"

namespace executable {

launcher_rvtm::launcher_rvtm() : _running(false) {
}

launcher_rvtm::~launcher_rvtm() {
}

int
launcher_rvtm::start_host() {
    return _launch("");
}

int
launcher_rvtm::start_client(const std::string &host_id) {
    return _launch(host_id);
}

int
launcher_rvtm::_launch(const std::string &host_id) {
    if (_running) return err_already_running;

    // First see if correct rvtm version has even been installed, huk!
    win_registry r(win_registry::id_uninstall, "", "RVTRM");

    if (!r.is_open()) {
        ACE_DEBUG((LM_INFO, "launcher_rvtm: uninstall registry entry not found\n"));
        return err_app_not_found;
    }
    std::string version = r.get<std::string>("DisplayVersion", "");

    // As a short cut, can use app_version_compare that is used with
    // RV House. RVTM's version numbering scheme seems to be reasonably
    // similar that it should work
    if (app_version_compare(version, "1.5.0") < 0) {
        ACE_DEBUG((LM_INFO, "launcher_rvtm: rvtm version '%s' less than 1.5.0\n",
                   version.c_str()));
        return err_app_not_found;
    }

    std::string uninst = r.get<std::string>("UninstallString", "");
    if (uninst.empty()) {
        return err_app_not_found;
    }

    ACE_DEBUG((LM_DEBUG, "launcher_rvtm: uninst string: %s\n",
               uninst.c_str()));

    // Parse the installation path, for example:
    // "C:\WINNT\Re-Volt Track Manager\uninstall.exe"
    // "/U:C:\Program Files\Acclaim Entertainment\RvTrm\Uninstall\irunin.xml"
    std::vector<std::string> res(1);
    if (regexp::match("/U:(.*)\\\\Uninstall\\\\irunin.xml", uninst, res.begin())) {
        ACE_DEBUG((LM_DEBUG, "launcher_rvtm: got match %s\n", res[0].c_str()));
    } else {
        return err_app_not_found;
    }
    std::string cmd(res[0]);
    cmd += "\\rvtrm.exe";
    cmd += (host_id.empty() ? " /host" : " /join " + host_id);

    ACE_DEBUG((LM_DEBUG, "launcher_rvtm: command line: %s\n",
               cmd.c_str()));

    // Launch options
    ACE_Process_Manager *pm = ACE_Process_Manager::instance();
    ACE_Process_Options opts;
    opts.command_line(cmd.c_str());
    int sret = pm->spawn(opts, this);
    ACE_DEBUG((LM_INFO, "launcher_rvtm: pid %d from thread %t, cmd: %s\n",
               sret, cmd.c_str()));
    if (sret == ACE_INVALID_PID) {
        ACE_ERROR((LM_ERROR, "launcher_rvtm: failed to launch: %s\n",
                   cmd.c_str()));
        return err_could_not_launch;
    }

    _running = true;

    return 0;
}

int
launcher_rvtm::handle_exit(ACE_Process *proc) {
    ACE_DEBUG((LM_INFO, "launcher_rvtm::handle_exit: called from thread %t\n"));
    // There's a potential risk here since handle_exit is called from
    // different thread than _launch. But since integer operations are
    // usually atomic and the risk is at most theoretical, no locking done.
    _running = false;
    gui_messenger()->send_msg(new message(message::rvtm_exited));
    return 0;
}

} // ns executable


