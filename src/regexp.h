#ifndef _REGEXP_H_
#define _REGEXP_H_

#include <string>

#include <fx.h>

#include "common.h"

namespace regexp {
    bool match(
        const char *pattern, 
        const char *subject, 
        FXint mode = REX_NORMAL
    );
    bool match(
        const std::string &pattern, 
        const std::string &subject, 
        FXint mode = REX_NORMAL
    );
        
    
    // Match case insensitive
    inline bool matchi(const std::string &p, const std::string &s) {
        return match(p, s, REX_ICASE);
    }
    // Match case insensitive verbatim (without magic chars)
    inline bool matchiv(const std::string &p, const std::string &s) {
        return match(p, s, REX_ICASE|REX_VERBATIM);
    }
    
    // Use this if there is need to capture parenthesed expressions,
    // up to 9 results possible
    template <typename OutIter>
    bool match(const std::string &pattern, const std::string &subject,
               OutIter results, FXint mode = REX_CAPTURE)
    {
        FXRex rex(pattern.c_str(), mode);
        FXint beg[9], end[9];
        
        if (rex.match(subject.c_str(), subject.size(), beg, end, mode, 9)) {
            for (int i = 1; i < 9; i++) {
                if (beg[i] < 0) break;
                
                *results++ = std::string(subject, beg[i], end[i] - beg[i]);
            }
            return true;
        }
        return false;   
    }
}

#endif //_REGEXP_H_
