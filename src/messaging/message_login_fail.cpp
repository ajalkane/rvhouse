#include "message_login_fail.h"

message_login_fail::message_login_fail(
  int t, 
  const std::string &status
) : message(t), _status(status) {}

message *
message_login_fail::duplicate() {
    return new message_login_fail(id(), status());
}
