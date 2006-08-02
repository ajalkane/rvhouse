#ifndef _EXECUTABLE_IMPL_LAUNCHER_DUMMY_H_
#define _EXECUTABLE_IMPL_LAUNCHER_DUMMY_H_

#include "../launcher.h"

namespace executable {

class launcher_dummy : public launcher {
public:
    virtual int start(const std::string &app) { return err_not_supported; }
    virtual int start_host(const std::string &app) { return err_not_supported;}
    virtual int start_client(const std::string &app, const std::string &host_id) {
        return err_not_supported;
    }
};

} // ns executable

#endif //_EXECUTABLE_IMPL_LAUNCHER_DUMMY_H_
