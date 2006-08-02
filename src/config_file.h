#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

#include <string>
#include <sstream>
#include <list>
#include <map>

#include <fx.h>

#include "common.h"

/* Wrapper around FX's configuration file with the relevant functions
 * needed by this application */
class config_file {
    FXSettings *_conf;
    std::string _parsed_file;
public:
    config_file();
    config_file(const config_file &o);
    virtual ~config_file();

    void parse(const std::string &file);
    void save(const std::string &file);
    // Saves to the last parsed file
    void save();

    config_file &operator=(const config_file &o);

    inline std::string get(const char *sec,
                               const char *ndx);
    template <class T>
    T get(const char *sec, const char *ndx, const T &def_val);
    template <class T>
    const T &set(const char *sec, const char *ndx, const T &val);

    size_t sections_as_list(std::list<std::string> &l);
    size_t section_as_map(const char *section, std::map<std::string,std::string> &m);
};

inline std::string
config_file::get(const char *sec, const char *ndx) {
    return _conf->readStringEntry(sec, ndx, "");
}
template <class T> T
config_file::get(const char *sec, const char *ndx, const T &def_val) {
    const char *valstr = _conf->readStringEntry(sec, ndx, NULL);
    if (valstr == NULL) return def_val;
    std::istringstream oss(valstr);
    T val = def_val;
    oss >> val;
    return val;
}

template <class T> const T &
config_file::set(const char *sec, const char *ndx, const T &val) {
    std::ostringstream oss;
    oss << val;
    ACE_DEBUG((LM_DEBUG, "config_file: setting [%s]/%s: %s\n",
               sec, ndx, oss.str().c_str()));
    _conf->writeStringEntry(sec, ndx, oss.str().c_str());
    return val;
}

#endif //_CONFIG_FILE_H_
