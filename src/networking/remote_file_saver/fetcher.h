#ifndef _NETWORKING_REMOTE_FILE_SAVER_H_
#define _NETWORKING_REMOTE_FILE_SAVER_H_

#include <string>

#include "../http/fetcher.h"

namespace networking {
namespace remote_file_saver {
// notify status enums
enum {
    status_ok = 1,
    status_error,
    status_last,
};

// A subclass of this class should be inherited by parties
// that want to know the result of a fetch
class notify {
public:
    virtual ~notify();

    virtual void saver_done(int status, const char *extra) = 0;
};

// Internal class that translates http responses to
// notify object
class http_handler : public http::handler {
    notify     *_target;
    bool        _notified;
    std::string _save_as;

    void _do_notify(int reason, const char *details);
    int  _save_content(const char *data, size_t len);
public:
    http_handler(notify *t, const std::string &save_as);
    virtual ~http_handler();

    virtual int handle_response(const http::response &resp);
    virtual int handle_error(int reason, const char *details);
    virtual int handle_close();
};

class fetcher {
    http::fetcher _http_fetcher;
public:
    fetcher();
    virtual ~fetcher();

    virtual void fetch(const std::string &remote_location,
                       const std::string &local_location,
                       notify *n);
};
} // remote_file_saver
} // ns networking

#endif //_NETWORKING_REMOTE_FILE_SAVER_H_
