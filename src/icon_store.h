#ifndef _ICON_STORE_H_
#define _ICON_STORE_H_

#include <fx.h>

#include <map>
#include <string>

#include "common.h"

class icon_store {
    FXApp  *_app;
    FXIcon *_icon_factory(const std::string &file);
    FXIcon *_missing_icon;
    
    typedef std::map<std::string, FXIcon *> _key_icon_map_type;
    _key_icon_map_type _key_icon_map;
public:
    icon_store(FXApp *app);
    virtual ~icon_store();
    
    // Returns true on success
    bool load(const char *key, const std::string &file);
    
    // scans the icons to use from a directory
    void scan_from_dir(const std::string dir, const char *file_base_name, ...);
    
    // Returns the named resource, or NULL
    FXIcon *get(const char *key);
};

#endif /*_ICON_STORE_H_*/
