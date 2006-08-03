#include <string.h>

#include "regexp.h"

#include <fx.h>

namespace regexp {

bool match(const char *pattern, const char *subject) {
    // For now using Fox's regexp facilities
    FXRex rex(pattern);
    // ACE_DEBUG((LM_DEBUG, "regexp::match2: %s to %s\n", pattern, subject));   
    ACE_DEBUG((LM_DEBUG, "regexp::match2: %s\n", pattern)); 
    return rex.match(subject, strlen(subject));
}

bool match(const std::string &pattern, const std::string &subject) {
    FXRex rex(pattern.c_str());
    // ACE_DEBUG((LM_DEBUG, "regexp::match: %s to %s\n", pattern.c_str(), subject.c_str()));
    ACE_DEBUG((LM_DEBUG, "regexp::match: %s\n", pattern.c_str()));
    return rex.match(subject.c_str()); // , subject.size());
}

}
