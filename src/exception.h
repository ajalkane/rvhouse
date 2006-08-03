#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_
/**
 * @file    exception.h
 * @date    Apr 9, 2005
 * @author  Arto Jalkanen
 * @brief   defines exceptions that might be thrown by this module
 *
 * Fatal errors will be thrown by this module as exceptions. This file
 * defines used exception classes. Each exception carries a code
 * and an additional error string.
 */

#include <exception>
#include <stdarg.h>
#include <stdio.h>
#include "common.h"

class exception : public std::exception {
private:
    const char *_str;
    int         _code;
public:
    exception(int code, const char *reason) :
      _str(reason), _code(code) {
    }
    virtual ~exception() throw();
    virtual const char *what() const throw() { return _str; }
    virtual int         code() const { return _code; }
};

class exceptionf : public ::exception {
private:
    char _str_store[256];
    
public:
    exceptionf(int code, const char *format, ...) 
        : exception(code, format) {
        va_list a;
        va_start(a, format);
        vsnprintf(_str_store, sizeof(_str_store)/sizeof(_str_store[0]), 
                  format, a);
        va_end(a);

        ACE_ERROR((LM_ERROR, "exception: %s\n", _str_store));           
    }
    virtual ~exceptionf() throw();
    virtual const char *what() const throw() { return _str_store; }
};
 
#endif //_EXCEPTION_H_
