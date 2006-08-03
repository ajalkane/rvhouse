#ifndef _UTIL_H_
#define _UTIL_H_

#include <string>

#include "os_util.h"

#define array_sizeof(a) (sizeof(a) / sizeof(a[0]))

/**
 * A dynamic cast for pointers that throws an exception if invalid
 * (non-castable) type is received. Note since operation is always
 * on pointer to pointer, the parametarized types is the class to cast
 * to, not the full pointer specification
 */
template <class Target, class Src>
inline Target *dynamic_ptr_cast(Src *src) {
    // A little trickery here, dynamic cast throws an exception with 
    // non-castable types when references are used
    Target &t = dynamic_cast<Target &>(*src);
    return &t;
};

/**
 * An UnaryFunc that deletes a pointer
 */
template <class T>
class delete_ptr {
public:
    inline void operator()(T *ptr) {
        delete ptr;
    }   
};

/**
 * An UnaryFunc that replaces returns the element itself, or replaces it
 * with another.
 */
template <class T>
class element_replace {
    T _f, _r;
public:
    inline element_replace(const T &find, const T &replace_with) 
      : _f(find), _r(replace_with) {}
    
    inline const T operator()(const T &e) {
        return (e == _f ? _r : e); 
    }   
};

/**
 * Restores variables value to what it was, unless release is called
 */
template <class T>
class variable_guard {
    T   &_var;
    T    _val;
    bool _released;
public:
    inline variable_guard(T &var) : _var(var), _val(var), _released(false) {}
    inline ~variable_guard() {
        if (!_released) _var = _val;
    }
    inline T &release() { _released = true; return _var; }
};

// Dummy implementation, maybe do more general when time
inline std::string rot13(const std::string &s) {
    std::string result;
    std::string::const_iterator i = s.begin();
    for (; i != s.end(); i++) {
        unsigned char c = *i + 13;
        // Force the resulting range to be between 32 - 122
        c %= 123;
        if (c < 32) c += 32;    
        result += c; // (*i + 13);
    }
    // ACE_DEBUG((LM_DEBUG, "rot13: %s -> %s\n", s.c_str(), result.c_str()));
    return result;                         
}

// Dummy implementation, maybe do more general when time
inline std::string derot13(const std::string &s) {
    std::string result;
    std::string::const_iterator i = s.begin();
    for (; i != s.end(); i++) {
        unsigned char c = *i - 13;
        // Force the resulting range to be between 32 - 122
        if (c < 32) c = 123 - (32 - c);
        result += c;
    }
    
    // ACE_DEBUG((LM_DEBUG, "derot13: %s -> %s\n", s.c_str(), result.c_str()));
    return result;                         
}

#endif //_UTIL_H_
