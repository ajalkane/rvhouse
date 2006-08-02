#include "fetcher.h"

namespace networking {
namespace http {

fetcher::fetcher() : _timeout(30) {
    _connector =
        new ACE_Connector<fetcher_handler, ACE_SOCK_CONNECTOR>(
            ACE_Reactor::instance());

}
fetcher::~fetcher() {
    delete _connector;
}

void
fetcher::fetch(const url &u, handler *h) {
    request req(u);
    fetch(req, h);
}

void
fetcher::fetch(const request &req, handler *h) {
    fetcher_handler     *fh = new fetcher_handler(req, h);
    const ACE_INET_Addr &a  = req.target_url().addr();

    // ACE_DEBUG((LM_DEBUG, "Trying to fetch '%s' from %s:%d\n",
    //          req.target_url().file().c_str(), a.get_host_addr(), a.get_port_number()));
    ACE_DEBUG((LM_DEBUG, "Trying to fetch from %s:%d\n",
               a.get_host_addr(), a.get_port_number()));

    ACE_DEBUG((LM_DEBUG, "http::fetcher: connecting\n"));
    ACE_Synch_Options conn_opts(ACE_Synch_Options::USE_REACTOR |
                                ACE_Synch_Options::USE_TIMEOUT,
                                _timeout);
    ACE_OS::last_error(0);
    if (_connector->connect(fh, a, conn_opts) == -1 &&
            ACE_OS::last_error() != EWOULDBLOCK)
    {
        ACE_ERROR((LM_ERROR, "%p\n", "fetcher::fetch connect"));
        throw exceptionf(0, "Could not connect to %s:%d",
                         a.get_host_addr(), a.get_port_number());
    }

    ACE_DEBUG((LM_DEBUG, "http::fetcher: returning\n"));
    return;
}

} // ns http
} // ns networking

