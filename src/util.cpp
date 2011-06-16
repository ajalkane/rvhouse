#include <ace/Log_Msg.h>

#include "util.h"

// Dummy implementation, maybe do more general when time
std::string rot13(const std::string &s) {
    std::string result;
    std::string::const_iterator i = s.begin();
    for (; i != s.end(); i++) {
        unsigned char c = *i + 13;
        // Force the resulting range to be between 33 - 122
        c %= 123;
        if (c < 33) c += 33;
        result += c; // (*i + 13);
    }
    ACE_DEBUG((LM_DEBUG, "rot13: %s -> %s\n", s.c_str(), result.c_str()));
    return result;                         
}

// Dummy implementation, maybe do more general when time
std::string derot13(const std::string &s) {
    std::string result;
    std::string::const_iterator i = s.begin();
    for (; i != s.end(); i++) {
        unsigned char c = *i - 13;
        // Force the resulting range to be between 33 - 122
        if (c < 33) c = 123 - (33 - c);
        result += c;
    }
    
    ACE_DEBUG((LM_DEBUG, "derot13: %s -> %s\n", s.c_str(), result.c_str()));
    return result;                         
}
