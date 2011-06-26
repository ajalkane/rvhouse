#ifndef CONFIG_MEMORY_H_
#define CONFIG_MEMORY_H_

#include <map>
#include <string>

#include "config.h"

/* A configuration in memory. */
class config_memory : public config {
    key_value_map_type _conf;

public:
    config_memory(const config &copy_from);

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

#endif //CONFIG_MEMORY_H_
