#ifndef _MESSAGING_MESSAGE_VERSION_H_
#define _MESSAGING_MESSAGE_VERSION_H_

#include <string>
#include <list>

#include "message.h"

class message_version : public message {
public:
    typedef std::list<std::string> list_type;

    message_version(int msg_type);
    virtual message *duplicate();


    inline const std::string &current() const         { return _current; }
    inline const std::string &current(const std::string &c) { return _current = c; }
    inline const std::string &current_url() const         { return _current_url; }
    inline const std::string &current_url(const std::string &c) { return _current_url = c; }
    inline const std::string &minimum() const { return _minimum; }
    inline const std::string &minimum(const std::string &c) { return _minimum = c; }

    inline const void whats_new_push_back(const std::string &s) {
        _whats_new.push_back(s);
    }
    inline size_t whats_new_size() const {
        return _whats_new.size();
    }
    inline list_type::const_iterator whats_new_begin() const {
        return _whats_new.begin();
    }
    inline list_type::const_iterator whats_new_end() const {
        return _whats_new.end();
    }

private:
    std::string _current;
    std::string _current_url;
    std::string _minimum;

    std::list<std::string> _whats_new;
};

#endif //_MESSAGING_MESSAGE_VERSION_H_
