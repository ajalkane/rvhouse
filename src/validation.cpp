#include "validation.h"
#include "regexp.h"

bool validate_user_id (const std::string &u) {
    if (u.size() < 3)  return false;
    if (u.size() > 20) return false;
    
    return regexp::match("^[a-zA-Z0-9_\\-\\[\\]=]+$", u);
    return regexp::match("^[a-zA-Z0-9\\[\\]_-=]+$", u);
    return regexp::match("^[a-zA-Z0-9_]+$", u);
}

bool validate_password(const std::string &p) {
    if (p.size() < 3)   return false;
    if (p.size() > 255) return false;
    
//    return regexp::match("^[\\w]+$", p);
    return regexp::match("^[a-zA-Z0-9_]+$", p);
}

bool validate_email   (const std::string &e) {
    return regexp::match(".+@.+\\..+", e);  
}
