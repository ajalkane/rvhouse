#ifndef MESSAGE_HANDLER_H_
#define MESSAGE_HANDLER_H_

#include "../messaging/message.h"

/**
 * Interface for classes that can handle internal messages
 */
class message_handler {
public:
    virtual void handle_message(message *msg) = 0;
};

#endif //MESSAGE_HANDLER_H_
