#include "message_version.h"

message_version::message_version(int t) : message(t) {}

message *
message_version::duplicate() {
    return new message_version(*this);
}
