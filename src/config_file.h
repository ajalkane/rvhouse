#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

#include <QSettings>

#include "config.h"
#include "common.h"
#include "util.h"

/* Wrapper around Qt's configuration file with the relevant functions
 * needed by this application */
class config_file : public config {
    QSettings   *_conf;
    bool        _no_delete;

public:
    enum config_type {
        ini_file = 0,
        registry
    };
    enum load_status {
        no_error = 0,
        access_error,
        format_error,
        does_not_exist_or_is_empty,
        other_error
    };

    config_file(config_type type);
    config_file(const config_file &o);

    virtual ~config_file();

    void load(const std::string &file);
    // Loads a configuration file conditionally, that is
    // if the file is not found it is not treated as error.
    // Returns load_status
    load_status load_conditionally(const std::string &file);
    void load();

    // Implemented from config
    // Returns a value for the given key
    virtual std::string get_value(const char *key) const;
    // Sets a value for the given key
    virtual void set_value(const char *key, const char *value);
    // Returns true if the configuration contains a value for the given key
    virtual bool contains(const char *key) const;
    // Call this ensures the changes are persisted
    virtual void save();

    typedef std::map<std::string, std::string> key_value_map_type;
    virtual size_t keys_and_values_as_map(key_value_map_type &m) const;
};

#endif //_CONFIG_FILE_H_
