#include "exception.h"

#include "win_registry.h"

#ifdef WIN32
#include <os_util.h>
#endif

win_registry::win_registry(const std::string &full_path) : _open(false)
{
    throw exception(0, "win_registry::full path ctor not implemented"); 
}

win_registry::win_registry(
    shortcut_id id, 
    const std::string &vendor,
    const std::string &app,
    const std::string &version
) : _open(false)
{
#ifdef WIN32
    HKEY root_key = HKEY_LOCAL_MACHINE;
    std::string path;
    switch (id) {
    case id_software:
        path = "SOFTWARE\\" + vendor + "\\" + app;
        if (!version.empty()) {
            path += "\\" + version;
        }
        break;
    case id_uninstall:
        path = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + app;
        break;
    case id_dplay:
        path = os::is_windows_64()
             ? "SOFTWARE\\Microsoft\\DirectPlay\\Applications\\Re-Volt"
             : "SOFTWARE\\Wow6432Node\\Microsoft\\DirectPlay\\Applications\\Re-Volt";        
    default:
        throw exceptionf(0, "win_registry::ctor unrecognized id %d", id);
    }
    
    LONG res = RegOpenKeyEx(
        root_key,
        path.c_str(),
        0, KEY_QUERY_VALUE | KEY_SET_VALUE, &_parent
    );

    if(res != ERROR_SUCCESS) {
        ACE_DEBUG((LM_DEBUG, "win_registry::ctor failed to open for "
                  "query/set access the key %s\n", path.c_str()));      
    } else {
        _open = true;
    }
#endif

}

win_registry::~win_registry() {
    ACE_DEBUG((LM_DEBUG, "win_registry::dtor\n"));
#ifdef WIN32
    if (_open) {
        ACE_DEBUG((LM_DEBUG, "win_registry::dtor closing parent\n"));
        RegCloseKey(_parent);
    }
#endif

}
