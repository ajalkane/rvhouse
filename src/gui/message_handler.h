#ifndef _MESSAGE_HANDLER_H_
#define _MESSAGE_HANDLER_H_

#include "../messaging/message.h"

/**
 * Interface for classes that can handle internal messages
 */
class message_handler {
public:
    virtual void handle_message(message *msg) = 0;
};

#endif //_MESSAGE_HANDLER_H_
