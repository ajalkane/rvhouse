#ifndef _VALIDATION_H_
#define _VALIDATION_H_

#include <string>

#include "common.h"

bool validate_user_id (const std::string &u);
bool validate_password(const std::string &p);
bool validate_email   (const std::string &e);

#endif //_VALIDATION_H_
