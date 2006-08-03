#ifndef _PARSER_URL_STATE_H_
#define _PARSER_URL_STATE_H_

#include <stddef.h>

namespace parser {
namespace url {

// Deterministic finite state machine
class dfa {
public:
    enum status {
        success = 1,
        failure
    };
    inline dfa() : _end(NULL) {}
    
    status parse(const char *buf, const char *end);
    
    inline const char *end() const { return _end; }
private:
    const char *_end;
    inline status _prepare_end(const char *begin, const char *end, status s);
};
    
} // ns url
} // ns parser

#endif //_PARSER_URL_STATE_H_
