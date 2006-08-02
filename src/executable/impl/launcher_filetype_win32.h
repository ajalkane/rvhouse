#ifndef _EXECUTABLE_IMPL_LAUNCHER_FILETYPE_WIN32_H_
#define _EXECUTABLE_IMPL_LAUNCHER_FILETYPE_WIN32_H_

#include <string>

#include "../launcher.h"

namespace executable {

class launcher_filetype_win32 : public launcher {
public:
    launcher_filetype_win32();
    virtual ~launcher_filetype_win32();

    virtual int start(const std::string &app);
    virtual int start_host(const std::string &app) { return err_not_supported;}
    virtual int start_client(const std::string &app, const std::string &host_id) {
        return err_not_supported;
    }
};

} // ns executable

#endif //_EXECUTABLE_IMPL_LAUNCHER_FILETYPE_WIN32_H_
