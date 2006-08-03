#ifndef _LANG_MAPPER_H_
#define _LANG_MAPPER_H_

#include <map>
#include <string>

#include <string.h>

namespace lang {

struct strless {
    inline bool operator()(const char *f, const char *s) const {
        return (strcmp(f, s) < 0 ? true : false);
    }
};

class mapper {
public:
    typedef std::map<const char *, const char *, strless> map_type;
    
private:
    map_type _map;
    map_type _map_def;
    map_type *_set_map;
    
    void _free_map(map_type &m);
public:
    enum set_type {
        set_normal = 1,
        set_default,
    };
    
    mapper();
    ~mapper();
    
    const char *get(const char *key) const;
    std::string get(const char *key, 
                    const char *r1, 
                    const char *r2 = NULL,
                    const char *r3 = NULL,
                    const char *r4 = NULL) const;
    // Concatenates a range of entries using a
    // separator
    std::string get_range(
        const char *key, 
        size_t start,
        size_t end,
        const char *separator,
        const char *r1 = NULL, 
        const char *r2 = NULL,
        const char *r3 = NULL,
        const char *r4 = NULL) const;

    // std::string map(const char *key, ...);
    
    void set_mode(set_type t);
    // Throws exception if error (duplicates etc.)
    void set(const char *key, const char *value);
    
    inline const map_type &get_map() const { return _map; }
};

}

#endif /*MAPPER_H_*/
