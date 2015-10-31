#ifndef _EXECUTABLE_IMPL_LAUNCHER_RVGL_H_
#define _EXECUTABLE_IMPL_LAUNCHER_RVGL_H_

#include <ace/Event_Handler.h>

#include <string>

#include "../launcher.h"

namespace executable {

class launcher_rvgl : public launcher, public ACE_Event_Handler {
    int  _launch(const std::string &host_id);
    volatile pid_t _rvgl_pid;
    volatile bool  _running;
public:
    launcher_rvgl();
    virtual ~launcher_rvgl();
    
    virtual int start_host();
    virtual int start_client(const std::string &host_id);   
    virtual int start(const std::string &app) { return err_not_supported; }
    virtual int start_host(const std::string &app) { return err_not_supported;}
    virtual int start_client(const std::string &app, const std::string &host_id) {
        return err_not_supported;
    }

    // ACE Event Handler interface for catching when the process
    // has exited.
    virtual int handle_exit(ACE_Process *proc);
};

} // ns executable

#endif //_EXECUTABLE_IMPL_LAUNCHER_RVGL_H_
