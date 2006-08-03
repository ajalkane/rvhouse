#include <utility>
#include <iostream>
#include <fstream>

#include "fetcher.h"

namespace networking {
namespace remote_file_saver {
    
notify::~notify() {}

/*************************************
 * http_handler
 *************************************/         

http_handler::http_handler(notify *t, const std::string &save_as) 
  : _target(t), _notified(false), _save_as(save_as)
{}
    
http_handler::~http_handler() {}

void
http_handler::_do_notify(int reason, const char *details) {
    _target->saver_done(reason, details);
    _notified = true;
}

int
http_handler::_save_content(const char *data, size_t len) {
    using std::ofstream;
    using std::ios;
    
    ofstream out(_save_as.c_str(), ios::out | ios::trunc | ios::binary);
    out.write(data, len);
    out.close();

    if (!out.good()) {
        ACE_DEBUG((LM_ERROR, "Error writing content to file %s\n",
                  _save_as.c_str()));
        return -1;
    }
    
    return 0;
}

int
http_handler::handle_response(const http::response &resp) {
    ACE_DEBUG((LM_DEBUG, "remote_file_saver: received content of length %d\n",
               resp.content_len()));
               
    if (!_save_content(resp.content(), resp.content_len()))
        _do_notify(status_ok, "response received");
    else
        _do_notify(status_error, "could not save content to file");
        
    return 0;
}

int 
http_handler::handle_error(int reason, const char *details) {
    _do_notify(status_error, details);
    return 0;
}

int 
http_handler::handle_close() {
    if (!_notified) {
        _do_notify(status_error, "Close received unexpectedly");
    }
    
    delete this;
    return 0;
}
        
/*************************************
 * fetcher
 *************************************/         
fetcher::fetcher() {
}
fetcher::~fetcher() {
}
        
void 
fetcher::fetch(const std::string &remote_location,
               const std::string &local_location,
               notify *n)
{
    if (remote_location.empty())
        throw exception(0, "remote_file_saver: remote_location empty");
    if (local_location.empty())
        throw exception(0, "remote_file_saver: local_location empty");
        
    // At the moment only http fetches are supported, so can assume
    // http urls
    http::url     remote_url(remote_location);
    http::request http_req(remote_url);
    
    auto_ptr<http_handler> h(new http_handler(n, local_location));
    _http_fetcher.fetch(http_req, h.get());
    h.release();
}

} // remote_file_saver  
} // ns networking
