#ifdef WIN32
#include <windows.h>
#else
#include <stdio.h>
#endif

#include "os_util.h"
#include "exception.h"
#include "app_version.h"

namespace os {
    
int alert(const char *topic, const char *text) {
#ifdef WIN32
    MessageBox(NULL, text, topic, MB_OK);
#else
    fprintf(stderr, "FATAL: %s: %s\n", topic, text);
#endif
    return 0;
}

namespace {
    void _app_dir(FXString *dir) {
    #ifdef WIN32
        TCHAR filepath[MAX_PATH];
    
        if( !GetModuleFileName(NULL, filepath, MAX_PATH ) ) {
            throw exceptionf(0, "GetModuleFileName failed (%d)\n", GetLastError()); 
        }
        *dir = FXFile::directory(filepath);
        ACE_DEBUG((LM_DEBUG, "os_app_dir: %s -> %s\n", filepath, dir->text()));
    #else
        *dir = FXFile::getCurrentDirectory();
    #endif
        
    }
}

const std::string &
app_dir() {
    // A static cache of the application directory for fast access
    static std::string app_dir_cache;
    
    if (app_dir_cache.empty()) {
        FXString d;
        _app_dir(&d);
        app_dir_cache = d.text();
    }
    
    return app_dir_cache;
}

void flash_window(FXWindow *win) {
#ifdef WIN32
    if (!win->hasFocus())
        FlashWindow((HWND)win->id(), TRUE);
#endif  
}

bool ensure_single_app_instance() {
#ifndef RV_HOUSE_TEST
#ifdef WIN32
    // This is not a very robust way to check if this is single 
    // running instance, but the hell with it, it's the easiest.
    // Amongst the bugs are that if there is ANY window open
    // with the name "RV House", then it thinks RV House is
    // running. Yes, this includes having the RV House installation
    // folder open in explorer!
    if (FindWindow(NULL, APP_NAME)) {
        return false;
    }
#endif
#endif
    return true;    
}

} // ns os
