#include <windows.h>

#include "launcher_filetype_win32.h"

namespace executable {

launcher_filetype_win32::launcher_filetype_win32() {
}

launcher_filetype_win32::~launcher_filetype_win32() {
}

int
launcher_filetype_win32::start(const std::string &app) {
    int ret = (int)ShellExecute(NULL,"open",app.c_str(),NULL,NULL,SW_SHOWNORMAL);
    ACE_DEBUG((LM_DEBUG, "launcher_filetype_win32::start return value %d for %s\n",
               ret, app.c_str()));
    // Returns value > 32 if successfull
    if (ret > 32) return 0;

    // Otherwise an error
    switch (ret) {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
        return err_app_not_found;
    case 0:
        return err_could_not_launch;
    }

    return err_other;
}

} // ns executable


