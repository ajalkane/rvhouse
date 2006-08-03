#include <vector>
#include "../../regexp.h"
#include "url.h"

namespace networking {
namespace http {

url::~url() {
    
}

void 
url::parse(const std::string &s) {
    // ACE_DEBUG((LM_DEBUG, "Parsing URL: %s\n", s.c_str()));
    ACE_DEBUG((LM_DEBUG, "Parsing URL\n"));
    
    // Very crude parser for parsing url string of style:
    // http://server/path/file  
    std::vector<std::string> res(3);
    if (!regexp::match("http:(\\d*)//([^/]+)(.*)", s, res.begin()))
        throw exceptionf(0, "Could not parse URL: %s", s.c_str());
    
    _port = res[0];
    _host = res[1];
    _file = res[2];
    
    if (_port.empty()) _port = "80";
    if (_file.empty()) _file = "/";
    
    // ACE_DEBUG((LM_DEBUG, "Parsed URL: %s to host: %s, port: %s, file: %s\n",
    //          s.c_str(), _host.c_str(), _port.c_str(), _file.c_str()));

    ACE_DEBUG((LM_DEBUG, "Parsed URL: to host: %s, port: %s\n",
               _host.c_str(), _port.c_str()));
            
    // std::string addr_str = _host + ":" + _port;
    // _addr.set   (addr_str.c_str());
    _addr.set(atoi(_port.c_str()), _host.c_str());
    
    if (_addr.is_any())  // _addr.get_ip_address() == INADDR_ANY ||
        // _addr.get_ip_address() == INADDR_NONE)
    {
        // throw exceptionf(0, "Could not resolve address");
        throw exceptionf(0, "Could not resolve address '%s' from URL: %s", 
                         _host.c_str(), s.c_str());
    }       
}


} // ns http
} // ns networking
