#include "message_register.h"

message_register::message_register(
    int t,
    const std::string &user,
    const std::string &pass,
    const std::string &mail
) : message_login(t, user, pass), _mail(mail) {}

message *
message_register::duplicate() {
    return new message_register(id(), user(), pass(), mail());
}
