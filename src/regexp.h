#ifndef _REGEXP_H_
#define _REGEXP_H_

#include <string>

#include <fx.h>

#include "common.h"

namespace regexp {
    bool match(const char *pattern, const char *subject);
    bool match(const std::string &pattern, const std::string &subject);
    
    // Use this if there is need to capture parenthesed expressions,
    // up to 9 results possible
    template <typename OutIter>
    bool match(const std::string &pattern, const std::string &subject,
               OutIter results)
    {
        FXRex rex(pattern.c_str(), REX_CAPTURE);
        FXint beg[9], end[9];
        
        if (rex.match(subject.c_str(), subject.size(), beg, end, REX_FORWARD, 9)) {
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
