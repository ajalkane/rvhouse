#ifndef _LANG_MANAGER_H_
#define _LANG_MANAGER_H_

#include <list>
#include <string>

#include "../common.h"
#include "info.h"
#include "mapper.h"

namespace lang {

class manager {
    std::list<info> _infos;

    void _mapper_fill(mapper &m, const std::string &name);
public:
    manager();
    virtual ~manager();
    
    void init(const std::string &lang_dir);
    bool language_exists(const std::string &name) const;
    
    inline const std::list<info> &lang_infos() { return _infos; }
    
    // Creates a language mapper... name is the language to use
    // and def is the default language to use for the mapper.
    void mapper_init(mapper &m, const std::string &name, const std::string &def);
};

} // ns lang
#endif /* _LANG_MANAGER_H_*/
