#ifndef _UTIL_H_
#define _UTIL_H_

#include <string>
#include <locale>

// #include "os_util.h"

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

std::string rot13(const std::string &s);
std::string derot13(const std::string &s);

// Utility struct for std::stream classes.
// Disables space as a white space separator. Needed so that
// can extract nicely also strings that have white spaces in them.
// Modified from the example found from here:
// http://www.talkaboutprogramming.com/group/comp.lang.c++/messages/740314.html
struct no_space_as_ws_ctype : std::ctype<char> {
    no_space_as_ws_ctype() : std::ctype<char>(get_table()) {}
    static std::ctype_base::mask const *get_table() {
        static std::ctype_base::mask *rc = 0;

        if (rc == 0) {
            rc = new std::ctype_base::mask[std::ctype<char>::table_size];
            std::fill_n(rc, std::ctype<char>::table_size,
                        std::ctype_base::mask());
            // Use only linefeed as a separator
            rc['\n'] = std::ctype_base::space;
        }
        return rc;
    }
};  

class char_array_copy {
    int strings_size;
    char **strings;
public:
    char_array_copy(int strings_size, char **strings) : strings_size(0), strings(NULL) {
        this->strings_size = strings_size;
        this->strings = new char *[strings_size];
        for (int i = 0; i < strings_size; ++i) {
            char *string = strdup(strings[i]);
            this->strings[i] = string;
        }
    }

    char_array_copy(int strings_size, const char **strings) : strings_size(0), strings(NULL) {
        this->strings_size = strings_size;
        this->strings = new char *[strings_size];
        for (int i = 0; i < strings_size; ++i) {
            char *string = strdup(strings[i]);
            this->strings[i] = string;
        }
    }

    ~char_array_copy() {
        for (int i = 0; i < strings_size; ++i) {
            free(this->strings[i]);
            this->strings[i] = NULL;
        }
        delete[] this->strings;
        this->strings = NULL;
    }

    char **array() {
        return strings;
    }

    int size() {
        return strings_size;
    }

};

#endif //_UTIL_H_
