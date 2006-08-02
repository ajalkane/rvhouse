#ifndef _EXECUTABLE_LAUNCHER_H_
#define _EXECUTABLE_LAUNCHER_H_

#include <string>

#include "../common.h"

namespace executable {

// Suports launching an executable. Several forms for launching,
// with each form supporting a default application to launch
// and a specific one. If one of the forms is not supported
// by implementation return an error or throw exception
class launcher {
protected:
    std::string _default_app;
public:
    enum {
        err_app_not_found = 1,
        err_could_not_launch,
        err_not_supported,
        err_already_running,
        err_other,
    };
    launcher();
    launcher(const std::string &default_app);
    virtual ~launcher();

    virtual int start();
    virtual int start(const std::string &app) = 0;
    virtual int start_host();
    virtual int start_host(const std::string &app) = 0;
    virtual int start_client(const std::string &host_id);
    virtual int start_client(const std::string &app, const std::string &host_id) = 0;

};

} // ns executable

#endif //_EXECUTABLE_LAUNCHER_H_
