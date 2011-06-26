#include "config_memory.h"

config_memory::config_memory(const config &copy_from)
{
    copy_from.keys_and_values_as_map(_conf);
}

std::string
config_memory::get_value(const char *key) const {
    key_value_map_type::const_iterator i = _conf.find(key);
    return i != _conf.end() ? i->second : std::string();
}

bool
config_memory::contains(const char *key) const {
    key_value_map_type::const_iterator i = _conf.find(key);
    return i != _conf.end();
}

void
config_memory::set_value(const char *key, const char *value) {
    _conf[key] = value;
}

void
config_memory::save() {
}


size_t
config_memory::keys_and_values_as_map(key_value_map_type &m) const {
    size_t items = 0;
    for (key_value_map_type::const_iterator i = _conf.begin(); i != _conf.end(); ++i) {
        m[i->first] = i->second;
        items++;
    }

    return items;
}
