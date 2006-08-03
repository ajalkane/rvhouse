#include "../exception.h"
#ifdef WIN32
#include "impl/launcher_dx7.h"
#include "impl/launcher_filetype_win32.h"
#include "impl/launcher_rvtm.h"
#else
#include "impl/launcher_filetype_unix.h"
#include "impl/launcher_dummy.h"
#endif
#include "factory.h"

namespace executable {

launcher *
factory::create_launcher(const std::string &app_id) {
    launcher *l = NULL;
    
#ifdef WIN32
    if (app_id == "Re-Volt")
        l = new launcher_dx7("Re-Volt");
    else if (app_id == "File")
        l = new launcher_filetype_win32;
    else if (app_id == "RVTM")
        l = new launcher_rvtm;
#else
    if (app_id == "Re-Volt")
        l = new launcher_dummy;
    else if (app_id == "File")
        l = new launcher_filetype_unix;
    else if (app_id == "Track Share")
        l = new launcher_dummy;
#endif
    
    if (!l) 
        throw exceptionf(0, "loppy_app::factory::create_launcher: "
        "not found for app id '%s'", app_id.c_str());
    
    return l;
};

} // ns executable
