#include <vector>

#include "../../messaging/message_global_ignore.h"
#include "../../util.h"
#include "../../regexp.h"
#include "../../app_version.h"

#include "../global.h"
#include "../worker.h"
#include "client.h"

namespace networking {
namespace global_ignore {

client::client() : _http_fetcher(NULL)
{   
}

client::~client() {
    delete _http_fetcher;
}
                
void 
client::fetch() {
    ACE_DEBUG((LM_DEBUG, "global_ignore::client::fetch\n"));
    std::string url = net_conf()->get<std::string>("main", "global_ignore", "");

    if (url.empty()) {
        ACE_DEBUG((LM_DEBUG, "global_ignore::client::fetch: empty URL\n"));
        delete this;
        return;
    }
    
    _http_fetcher = new http::fetcher;
    
    // HTTP fetcher throws an exception in case of an error
    try {
        ACE_DEBUG((LM_DEBUG, "global_ignore::client::fetch: "
                  "getting version information from %s\n",
                  url.c_str()));
        _http_fetcher->fetch(url, this);
    } catch (std::exception &e) {
        ACE_ERROR((LM_ERROR, "global_ignore::client::fetch: exception: %s\n",
                  e.what()));
        delete this;
        return;
    }
    
    // If gotten this far, deleted from handle_close
}

// http::handler interface      
int
client::handle_response(const http::response &resp) 
{
    ACE_DEBUG((LM_DEBUG, "global_ignore::client::got response: %s\n",
              resp.content()));
    const char *str_s = resp.content();
    const char *str_e = strchr(str_s, '\n');
    
    message_global_ignore *m = new message_global_ignore(message::global_ignore_list);
    auto_ptr<message_global_ignore> m_guard(m);
    // Go through the file line by line
    for (; str_e; str_s = str_e + 1, str_e = strchr(str_s, '\n')) {
        if (str_e > str_s && *(str_e-1) == '\r') --str_e;
        std::string line(str_s, (int)(str_e - str_s));
        std::vector<std::string> res(3);

        if (regexp::match("^ip;([^;]+);([^;]+);([^;]*)", line, res.begin())) {
            ACE_DEBUG((LM_DEBUG, "global_ignore::client::got ignore entry "
                      "IP/userid/reason: %s/%s/%s\n",
                      res[0].c_str(),
                      res[1].c_str(),
                      res[2].c_str()
                      ));
            message_global_ignore::ip_entry e;
            e.ip     = res[0];
            e.userid = res[1];
            e.reason = res[2];
            m->ip_push_back(e);
        } else if (line.empty()) {
            // break;
        }
    }

    if (m->size() > 0) {
        gui_messenger()->send_msg(m);
        m_guard.release();
    }
    return 0;
}

int
client::handle_error(int reason, const char *details) {
    ACE_DEBUG((LM_DEBUG, "global_ignore::client: error: %s\n", details));
    return 0;   
}

int
client::handle_close() {
    ACE_DEBUG((LM_DEBUG, "global_ignore::client: destroying handler\n"));
    delete this;
    ACE_DEBUG((LM_DEBUG, "global_ignore::client: destroying handler, done\n"));
    return 0;   
}
    
} // ns global_ignore
} // ns networking
