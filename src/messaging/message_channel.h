#ifndef _MESSAGING_MESSAGE_CHANNEL_H_
#define _MESSAGING_MESSAGE_CHANNEL_H_

#include <string>

#include "message_send.h"

class message_channel : public message_send {
    std::string _channel;
public:
    message_channel(int msg_type,
                    const std::string &str,
                    const chat_gaming::user::id_type &uid,
                    const std::string &channel,
                    unsigned seq,
                    int group_base);

    virtual message *duplicate();

    inline const std::string &channel() const;
};

inline const std::string &message_channel::channel() const { return _channel; }

#endif //_MESSAGING_MESSAGE_CHANNEL_H_
