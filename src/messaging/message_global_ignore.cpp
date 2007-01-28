#include "message_global_ignore.h"

message_global_ignore::message_global_ignore(int t) : message(t) {}

message *
message_global_ignore::duplicate() {
    return new message_global_ignore(*this);
}
