#include "message.h"

message::message(int t) : _id(t) {}
message::~message() {}

message *
message::duplicate() {
    return new message(_id);
}
