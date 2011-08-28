#ifndef _VALIDATION_H_
#define _VALIDATION_H_

#include <string>

#include "common.h"

bool validate_user_id (const std::string &u, bool no_size_validation = false);
bool validate_password(const std::string &p, bool no_size_validation = false);
bool validate_email   (const std::string &e, bool no_size_validation = false);

#endif //_VALIDATION_H_
