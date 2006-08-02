#ifndef _FILE_UTIL_H_
#define _FILE_UTIL_H_

#include <string>

#include "common.h"

std::string app_rel_path(const char *path);
std::string app_rel_path(const std::string &path);
bool file_exists(const std::string &fpath);
bool file_delete(const std::string &fpath);

#endif //_FILE_UTIL_H_
