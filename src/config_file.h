#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

#include <string>
#include <sstream>
#include <list>
#include <locale>
#include <map>

#include <QSettings>

#include "common.h"
#include "util.h"

/* Wrapper around Qt's configuration file with the relevant functions
 * needed by this application */
class config_file {
    QSettings   *_conf;
    bool        _no_delete;

    // static no_space_as_ws_ctype *_loc_facet;
    static std::locale _loc;

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
    void save();

    config_file &operator=(const config_file &o);

    inline std::string get(const char *key) const;
    template <class T>
    T get(const char *key, const T &def_val) const;
    // Convenience method
    template <class T>
    T get(const char *sec, const char *sec_key, const T &def_val) const;

    // Template specialization for std::strings
    /*std::string get(
        const char *sec, const char *ndx,
        const std::string &def_val);
    */
    template <class T>
    const T &set(const char *key, const T &val);
    // Convenience method
    template <class T>
    const T &set(const char *sec, const char *sec_key, const T &val);

    typedef std::map<std::string, std::string> key_value_map_type;
    size_t keys_and_values_as_map(key_value_map_type &m) const;

    // size_t sections_as_list(std::list<std::string> &l);
    // size_t section_as_map(const char *section, std::map<std::string,std::string> &m);
};

inline std::string
config_file::get(const char *key) const {
    QVariant v_value = _conf->value(key, "");
    return std::string(v_value.toString().toLatin1().constData());
}

/*
inline std::string
config_file::get(const char *sec, const char *ndx, const std::string &def_val) {
    return _conf->readStringEntry(sec, ndx, def_val.c_str());
}
*/

template <class T> T
config_file::get(const char *key, const T &def_val) const {
    if (!_conf->contains(key)) return def_val;
    std::string value = get(key);
    std::istringstream oss(value);
    oss.imbue(_loc);
    T val = def_val;
    oss >> val;
    return val;
}

template <class T> T
config_file::get(const char *sec, const char *sec_key, const T &def_val) const {
    std::string key = sec;
    key += '/';
    key += sec_key;
    return get<T>(key.c_str(), def_val);
}

template <class T> const T &
config_file::set(const char *key, const T &val) {
    std::ostringstream oss;
    oss << val;
    ACE_DEBUG((LM_DEBUG, "config_file: setting [%s]: %s\n",
              key, oss.str().c_str()));
    _conf->setValue(key, QVariant(oss.str().c_str()));
    return val;
}

template <class T> const T &
config_file::set(const char *sec, const char *sec_key, const T &val) {
    std::string key = sec;
    key += '/';
    key += sec_key;
    return set(key.c_str(), val);
}

#endif //_CONFIG_FILE_H_
