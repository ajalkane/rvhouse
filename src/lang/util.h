#ifndef _LANG_UTIL_H_
#define _LANG_UTIL_H_

#include "info.h"

namespace lang {

class info_match_lang_fobj {
    std::string _lang;  
public:
    inline info_match_lang_fobj(const std::string &match) : _lang(match) {}
    inline bool operator()(const info &i) const { return _lang == i.lang(); }
};

} // ns lang

#endif /*UTIL_H_*/
