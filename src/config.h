#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string>
#include <sstream>
#include <list>
#include <locale>
#include <map>

#include <ace/Log_Msg.h>

/* Abstract class for configuration. The implementing class has to implement few
 * functions to provide the full operation
 * needed by this application */
class config {
    // static no_space_as_ws_ctype *_loc_facet;
    static std::locale _loc;

public:
    virtual ~config();

    // Returns a value for the given key
    virtual std::string get_value(const char *key) const = 0;
    // Sets a value for the given key
    virtual void set_value(const char *key, const char *value) = 0;
    // Returns true if the configuration contains a value for the given key
    virtual bool contains(const char *key) const = 0;
    // Call this ensures the changes are persisted
    virtual void save() = 0;

    typedef std::map<std::string, std::string> key_value_map_type;
    virtual size_t keys_and_values_as_map(key_value_map_type &m) const = 0;

    template <class T>
    T get(const char *key, const T &def_val) const;
    // Convenience method
    template <class T>
    T get(const char *sec, const char *sec_key, const T &def_val) const;

    template <class T>
    const T &set(const char *key, const T &val);
    // Convenience method
    template <class T>
    const T &set(const char *sec, const char *sec_key, const T &val);

};

template <class T> T
config::get(const char *key, const T &def_val) const {
    if (!contains(key)) return def_val;
    std::string value = get_value(key);
    std::istringstream oss(value);
    oss.imbue(_loc);
    T val = def_val;
    oss >> val;
    return val;
}

template <class T> T
config::get(const char *sec, const char *sec_key, const T &def_val) const {
    std::string key = sec;
    key += '/';
    key += sec_key;
    return get<T>(key.c_str(), def_val);
}

template <class T> const T &
config::set(const char *key, const T &val) {
    std::ostringstream oss;
    oss << val;
    ACE_DEBUG((LM_DEBUG, "config: setting [%s]: %s\n",
              key, oss.str().c_str()));
    set_value(key, oss.str().c_str());
    return val;
}

template <class T> const T &
config::set(const char *sec, const char *sec_key, const T &val) {
    std::string key = sec;
    key += '/';
    key += sec_key;
    return set(key.c_str(), val);
}

#endif //_CONFIG_H_
