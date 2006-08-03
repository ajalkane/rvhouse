#ifndef _COMMON_H_
#define _COMMON_H_

#include <ace/Log_Msg.h>

#include "main.h"

// Language translations are so common occurence that
// include needed prototypes and specify functions here
#include "lang/mapper.h"

inline const char *langstr(const char *key) {
    return lang_mapper.get(key);
}

inline std::string langstr(
  const char *key, 
  const char *r1,
  const char *r2 = NULL,
  const char *r3 = NULL,
  const char *r4 = NULL
) 
{
    return lang_mapper.get(key, r1, r2, r3, r4);
}

inline std::string langstr_range(
  const char *key, 
  size_t start,
  size_t end,
  const char *separator,
  const char *r1 = NULL,
  const char *r2 = NULL,
  const char *r3 = NULL,
  const char *r4 = NULL
) 
{
    return lang_mapper.get_range(key, start, end, separator, r1, r2, r3, r4);
}

#endif //_COMMON_H_
