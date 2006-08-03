#include <vector>

#include "../../messaging/message_version.h"
#include "../../util.h"
#include "../../regexp.h"
#include "../../app_version.h"

#include "../global.h"
#include "../worker.h"
#include "client.h"

namespace networking {
namespace version_update {

client::client() : _http_fetcher(NULL)
{   
}

client::~client() {
    delete _http_fetcher;
}
                
void 
client::fetch() {
    ACE_DEBUG((LM_DEBUG, "version_update::client::fetch\n"));
    std::string url = net_conf()->get<std::string>("main", "version_update", "");

    if (url.empty()) {
        ACE_DEBUG((LM_DEBUG, "version_update::client::fetch: empty URL\n"));
        delete this;
        return;
    }
    
    _http_fetcher = new http::fetcher;
    
    // HTTP fetcher throws an exception in case of an error
    try {
        ACE_DEBUG((LM_DEBUG, "version_update::client::fetch: "
                  "getting version information from %s\n",
                  url.c_str()));
        _http_fetcher->fetch(url, this);
    } catch (std::exception &e) {
        ACE_ERROR((LM_ERROR, "version_update::client::fetch: exception: %s\n",
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
    ACE_DEBUG((LM_DEBUG, "version_update::client::got response: %s\n",
              resp.content()));
    const char *str_s = resp.content();
    const char *str_e = strchr(str_s, '\n');
    
    message_version *m = new message_version(message::version);
    auto_ptr<message_version> m_guard(m);
    // Go through the file line by line
    for (; str_e; str_s = str_e + 1, str_e = strchr(str_s, '\n')) {
        std::string line(str_s, (int)(str_e - str_s));
        std::vector<std::string> res(3);

        if (regexp::match("^- (.*)$", line, res.begin())) {
            ACE_DEBUG((LM_DEBUG, "version_update::client::got whats new: %s\n",
                      res[0].c_str()));
            if (m->whats_new_size() >= 10) {
                m->whats_new_push_back("... and more");
                break;
            }                     
            m->whats_new_push_back(res[0]);         
        } else if (regexp::match("^version:(.+) +min:(.*) +(http[^ ]+)$", line, res.begin())) {
            ACE_DEBUG((LM_DEBUG, "version_update::client::got: "
            "version %s, min: %s, link: %s\n",
            res[0].c_str(),
            res[1].c_str(),
            res[2].c_str()));
    
            if (m->current().empty() &&
                app_version_compare(APP_VERSION, res[0]) < 0) 
            {       
                m->current(res[0]);
                m->current_url(res[2]);
                m->minimum(res[1]);

                ACE_DEBUG((LM_DEBUG, "version_update::client: "
                "new version available, our version %s, current %s\n",
                APP_VERSION, m->current().c_str()));

                if (app_version_compare(APP_VERSION, m->minimum()) < 0) {
                    ACE_DEBUG((LM_DEBUG, "version_update::client: "
                    "must update version, our version %s, minimum %s\n",
                    APP_VERSION, m->minimum().c_str()));
                    m->id(message::version_force);
                }
            } else if (app_version_compare(APP_VERSION, res[0]) >= 0) {
                break;
            }
        } else if (line.empty()) {
            // break;
        }
    }

    if (!m->current().empty()) {
        gui_messenger()->send_msg(m);
        m_guard.release();
    }
    return 0;
}

int
client::handle_error(int reason, const char *details) {
    ACE_DEBUG((LM_DEBUG, "version_update::client: error: %s\n", details));
    return 0;   
}

int
client::handle_close() {
    ACE_DEBUG((LM_DEBUG, "version_update::client: destroying handler\n"));
    delete this;
    ACE_DEBUG((LM_DEBUG, "version_update::client: destroying handler, done\n"));
    return 0;   
}
    
} // ns version_update
} // ns networking
