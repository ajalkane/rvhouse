#include "../../common.h"
#include "dfa.h"

namespace parser {
namespace url {
    
namespace {
    // deterministic finite state machine state transition table
    // recognizes URL:s of the form:
    // ftp://blabla
    // http://balblab
    // www.blabla.doh
    const int TOKEN_H     = 0;
    const int TOKEN_T     = 1;
    const int TOKEN_P     = 2;
    const int TOKEN_COLON = 3;
    const int TOKEN_SLASH = 4;
    const int TOKEN_F     = 5;
    const int TOKEN_W     = 6;
    const int TOKEN_DOT   = 7;
    const int TOKEN_TERM  = 8;
    const int TOKEN_OTHER = 9;

    const int TOKEN_REMOVE_END = 10;
    
    int _table[][11] =
//   h    t   p   :  /   f    w,  . TERM  OTHER REMOVE_END
{ 
   // Start rule
   {  1, -1, -1, -1, -1, 2,  7, -1,  -1, -1, -1  },
   // Rule for http
   { -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1  },
   { -1,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1  },
   { -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1  },
   // Rule for start of ://
   { -1, -1, -1,  5, -1, -1, -1, -1, -1, -1, -1  },
   { -1, -1, -1, -1,  6, -1, -1, -1, -1, -1, -1  },
   { -1, -1, -1, -1, 10, -1, -1, -1, -1, -1, -1  },
   // Rule for www.something.what
   { -1, -1, -1, -1, -1, -1,  8, -1, -1, -1, -1  },
   { -1, -1, -1, -1, -1, -1,  9, -1, -1, -1, -1  },
   { -1, -1, -1, -1, -1, -1, -1, 10, -1, -1, -1  },
   
   // Rule for the URL portion, basically anything goes now
   {  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 },
};

    inline int tokenize(char a) {
        switch(a) {
        case 'h': return TOKEN_H;
        case 't': return TOKEN_T;
        case 'p': return TOKEN_P;
        case ':': return TOKEN_COLON;
        case '/': return TOKEN_SLASH;
        case 'f': return TOKEN_F;
        case 'w': return TOKEN_W;
        case '.': return TOKEN_DOT;
            
        case ' ': return TOKEN_TERM;
        case '\t': return TOKEN_TERM;
        case '\n': return TOKEN_TERM;
        case '\r': return TOKEN_TERM;
        case '\0': return TOKEN_TERM;

        case ')': case '(':
        case '{': case '}':
        case '[': case ']':
        case ',': case ';':
            return TOKEN_REMOVE_END;        
        }
        return TOKEN_OTHER;
    }
}

inline dfa::status
dfa::_prepare_end(const char *begin, const char *end, dfa::status s)
{
    _end = end;
    for (; _end > begin; --_end) {
        switch (tokenize(*_end)) {
        case TOKEN_REMOVE_END:
        case TOKEN_TERM:
        case TOKEN_DOT:
            continue;
        }
        break;
    }
    _end++;
    return s;
}

dfa::status
dfa::parse(const char *buf, const char *guard) {
    ACE_DEBUG((LM_DEBUG, "dfa::parse: buf %s\n", buf));
    const char *begin = buf;
    int state = 0;
    int token = TOKEN_TERM;
    for (; buf < guard; buf++) {
        token = tokenize(*buf);
        ACE_DEBUG((LM_DEBUG, "dfa::parse: state/token %d/%d\n", state, token));
        if (token == TOKEN_TERM) {
            return _prepare_end(begin, buf, 
                   _table[state][token] == -1 ? failure : success);
        }
        state = _table[state][token];
        if (state == -1) {
            ACE_DEBUG((LM_DEBUG, "dfa::parse: returning failure\n", state, token));
            return _prepare_end(begin, buf, failure);
        }
    }
    ACE_DEBUG((LM_DEBUG, "dfa::parse: at end, state %d\n", state));
    return _prepare_end(begin, buf, 
                        _table[state][TOKEN_TERM] == -1 ? failure : success);
}

} // ns url
} // ns parser
