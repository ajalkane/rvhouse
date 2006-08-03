#include "message_login.h"

message_login::message_login(
  int t, 
  const std::string &user,
  const std::string &pass
) : message(t), _user(user), _pass(pass) {}

message *
message_login::duplicate() {
    return new message_login(id(), user(), pass());
}
