#ifndef _EXECUTABLE_IMPL_LAUNCHER_FILETYPE_UNIX_H_
#define _EXECUTABLE_IMPL_LAUNCHER_FILETYPE_UNIX_H_

#include <string>

#include "../launcher.h"

namespace executable {

class launcher_filetype_unix : public launcher {
    int _launch(const std::string &file);
public:
    launcher_filetype_unix();
    virtual ~launcher_filetype_unix();
    
    virtual int start(const std::string &app);
    virtual int start_host(const std::string &app) { return err_not_supported;}
    virtual int start_client(const std::string &app, const std::string &host_id) {
        return err_not_supported;
    }
};

} // ns executable

#endif //_EXECUTABLE_IMPL_LAUNCHER_FILETYPE_UNIX_H_
