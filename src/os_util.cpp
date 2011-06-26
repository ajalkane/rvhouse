#ifdef WIN32
#include <windows.h>
#include <tchar.h>
#else
#include <stdio.h>
#endif

#include <QApplication>
#include <QDir>
#include <QString>

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
    void _app_dir(QString *dir) {
        // IMPROVE Qt: using QApplication::applicationDirPath() would work on
        // all platforms, but it will require refactoring of the code so that
        // QApplication is instantiated always before using this method
        // before constructing (rvhouse.ini configuration reader for example)
    #ifdef WIN32
        TCHAR filepath[MAX_PATH];
    
        if( !GetModuleFileName(NULL, filepath, MAX_PATH ) ) {
            throw exceptionf(0, "GetModuleFileName failed (%d)\n", GetLastError()); 
        }
        QFileInfo file_info(filepath);
        *dir = file_info.absoluteDir().absolutePath();
        ACE_DEBUG((LM_DEBUG, "os_app_dir: %s -> %s\n", filepath, dir->toLatin1().constData()));
    #else
        *dir = QDir::currentPath();
    #endif
    }
}

const std::string &
app_dir() {
    // A static cache of the application directory for fast access
    static std::string app_dir_cache;
    
    if (app_dir_cache.empty()) {
        QString d;
        _app_dir(&d);
        app_dir_cache = d.toLatin1().constData();
    }
    
    return app_dir_cache;
}

void flash_window(QWidget *obj) {
    QApplication::alert(obj);
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

bool is_windows_64() {
#ifdef WIN32
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

    LPFN_ISWOW64PROCESS fnIsWow64Process;
    BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            //handle error
        }
    }
    return bIsWow64;
#endif
    return false;
}

bool is_windows_vista_or_later() {
#ifdef WIN32
    OSVERSIONINFO osvi;
    BOOL bIsWindowsXPorLater;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);

    return osvi.dwMajorVersion >= 6;
#endif
    return false;
}

} // ns os
