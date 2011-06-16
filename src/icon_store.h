#ifndef _ICON_STORE_H_
#define _ICON_STORE_H_

#include <QIcon>
#include <QApplication>

#include <map>
#include <string>

#include "common.h"

class icon_store {
    QApplication  *_app;
    QIcon *_icon_factory(const std::string &file);
    QIcon *_missing_icon;
    
    typedef std::map<std::string, QIcon *> _key_icon_map_type;
    _key_icon_map_type _key_icon_map;
public:
    icon_store(QApplication *app);
    virtual ~icon_store();
    
    // Returns true on success
    bool load(const char *key, const std::string &file);
    
    // scans the icons to use from a directory
    void scan_from_dir(const std::string dir, const char *file_base_name, ...);
    
    // Returns the named resource, or NULL
    const QIcon &get(const char *key) const;
};

#endif /*_ICON_STORE_H_*/
