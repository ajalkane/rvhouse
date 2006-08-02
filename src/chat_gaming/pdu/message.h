#ifndef _CHAT_GAMING_PDU_MESSAGE_H_
#define _CHAT_GAMING_PDU_MESSAGE_H_

#include <boost/serialization/access.hpp>

#include <string>

namespace chat_gaming {
namespace pdu {
class message {
    std::string _channel;
    std::string _data;

    friend class boost::serialization::access;

    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & _channel;
        ar & _data;
    }

public:
    inline message();
    inline message(const std::string &data);
    inline message(const std::string &data, const std::string &channel);

    inline const std::string &data(const std::string &);
    inline const std::string &data() const;

    inline const std::string &channel(const std::string &);
    inline const std::string &channel() const;
};

inline message::message() {}
inline message::message(const std::string &data)
        : _data(data) {}
inline message::message(const std::string &data, const std::string &channel)
        : _channel(channel), _data(data) {}

inline const std::string &
message::data(const std::string &s) { return _data = s; }
inline const std::string &
message::data() const { return _data; }

inline const std::string &
message::channel(const std::string &s) { return _channel = s; }
inline const std::string &
message::channel() const { return _channel; }
} // ns pdu
} // ns chat_gaming

#endif //_CHAT_GAMING_USER_H_
