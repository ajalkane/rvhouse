#include "message_string.h"

message_string::message_string(int t, const std::string &s) 
  : message(t), _str(s) {}

message *
message_string::duplicate() {
    return new message_string(id(), _str);
}
