#include <sstream>

#include <string.h>
#include <stdlib.h>
#include "../exception.h"

#include "mapper.h"

namespace lang {

mapper::mapper()
        : _map(strless()),
        _map_def(strless())
{
    _set_map = &_map;
}

mapper::~mapper() {
    _free_map(_map);
    _free_map(_map_def);
}

void
mapper::_free_map(map_type &m) {
    map_type::iterator i = m.begin();
    for (; i != m.end(); i++) {
        if (i->first)  free((void *)i->first);
        if (i->second) free((void *)i->second);
    }
    m.clear();
}

void
mapper::set_mode(set_type t) {
    switch (t) {
    case set_normal:  _set_map = &_map;     break;
    case set_default: _set_map = &_map_def; break;
    }
}

void
mapper::set(const char *key, const char *value) {
    map_type::iterator i = _set_map->find(key);
    if (i != _set_map->end())
        throw exceptionf(0, "Duplicate values in language file for key: %s",
                         key);
    (*_set_map)[strdup(key)] = strdup(value);
    ACE_DEBUG((LM_DEBUG, "mapper::set(%s, %s)\n", key, value));
}

const char *
mapper::get(const char *key) const {
        map_type::const_iterator i = _map.find(key);
        if (i == _map.end()) {
            if (_map_def.size() > 0) i = _map_def.find(key);
            else return key;
            if (i == _map_def.end()) return key;
        }
        return i->second;
    }

// Replaces some parameters from the string
std::string
mapper::get(
        const char *key,
        const char *r1,
        const char *r2,
        const char *r3,
        const char *r4) const
    {
        const char *val = get(key);
        bool quit = false;
        std::string res;
        while (!quit) {
            switch (*val) {
            case 0: quit = true; break;
            case '\\':
                {
                    val++;
                    // Substitute
                    const char *subs = NULL;
                    switch (*val) {
                    case 0  : quit = true; break;
                    case '1': subs = r1;   break;
                    case '2': subs = r2;   break;
                    case '3': subs = r3;   break;
                    case '4': subs = r4;   break;
                    }

                    if (subs)
                        res += subs;
                }
                break;
            default:
                res += *val;
            }
            val++;
        }

        return res;
    }

std::string
mapper::get_range(
    const char *key,
    size_t start,
    size_t end,
    const char *separator,
    const char *r1,
    const char *r2,
    const char *r3,
    const char *r4) const
{
    std::string res;
    std::string val;
    std::ostringstream fullkey;
    for (; start <= end; start++) {
        fullkey.str("");
        fullkey << key << start;
        val = get(fullkey.str().c_str(), r1, r2, r3, r4);
        if (!val.empty() && !res.empty())
            res.append(separator);
        if (!val.empty()) res.append(val);

    }
    return res;
}

} // ns lang

