#include "validation.h"
#include "regexp.h"

bool validate_user_id (const std::string &u, bool no_size_validation) {
    if (!no_size_validation && u.size() < 4) return false;
    //Hmm really no upper limit?
    //if (u.size() > 20) return false;
    
    return regexp::match("^[a-zA-Z0-9_\\-\\[\\]=]+$", u);
}

bool validate_password(const std::string &p, bool no_size_validation) {
    if (!no_size_validation && p.size() < 6) return false;

    //Hmm really no upper limit?
    //if (p.size() > 255) return false;
    
    return regexp::match("^[a-zA-Z0-9_]+$", p);
}

bool validate_email   (const std::string &e, bool no_size_validation) {
    if (!no_size_validation && e.size() < 7) return false;
    return regexp::match(".+@.+\\..+", e);  
}
