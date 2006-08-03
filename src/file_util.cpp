#include <string>
#include <algorithm>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include "common.h"
#include "os_util.h"
#include "util.h"

std::string 
app_rel_path(const char *path) {
    std::string apprel = path;
    // First must change path separation character to the one OS uses
    if (PATHSEP != '/') {
        std::transform(apprel.begin(), apprel.end(),
                       apprel.begin(), element_replace<char>('/', PATHSEP));
                       
        ACE_DEBUG((LM_DEBUG, "Transformed path %s -> %s\n", path, apprel.c_str()));
    }
    
    apprel.insert(0, 1, PATHSEP);
    apprel.insert(0, os::app_dir());

    ACE_DEBUG((LM_DEBUG, "Returning app_rel_path %s\n", apprel.c_str()));
    
    return apprel;
}

std::string 
app_rel_path(const std::string &path) {
    return app_rel_path(path.c_str());
}

bool file_exists(const std::string &fpath) {
    struct stat sbuf;
    return stat(fpath.c_str(), &sbuf) == 0;
}

bool file_delete(const std::string &fpath) {
    ACE_DEBUG((LM_DEBUG, "Removing file: %s\n", fpath.c_str()));
    
    if (remove(fpath.c_str())) {
        ACE_DEBUG((LM_WARNING, "Could not remove file: %s\n", fpath.c_str()));
        return false;
    }
    return true;
}
