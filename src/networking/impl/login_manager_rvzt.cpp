#include <exception>
#include <vector>

#include "../../messaging/message_login.h"
#include "../../messaging/message_login_fail.h"
#include "../../messaging/message_register.h"
#include "../../util.h"
#include "../../regexp.h"
#include "../http/url.h"
#include "../global.h"
#include "../worker.h"
#include "login_manager_rvzt.h"

namespace networking {

#define LOGIN_MESSAGE(msg) (msg - message::login + _msg_base)

login_manager_rvzt::login_manager_rvzt(ACE_Reactor *r)
  : login_manager(r),
    _http_fetcher(new http::fetcher)
{
    _base_validate_url = net_conf()->get<std::string>(
        "auth", "validate", "http://rvzt.zackattackgames.com/main/user_validate.php"
    );
    _base_register_url = net_conf()->get<std::string>(
        "auth", "register", "http://rvzt.zackattackgames.com/main/user_register.php"
    );

    // Ensure only expected authentiraztion urls are used
    ACE_ERROR((LM_DEBUG, "login_manager_rvzt: _base_validate_url %s)\n",
              _base_validate_url.c_str()));

    http::url validate_url = http::url(_base_validate_url);
    http::url register_url = http::url(_base_register_url);

    if (!_check_auth_url(validate_url))
        throw exception(0, "Invalid validation URL");
    if (!_check_auth_url(register_url))
        throw exception(0, "Invalid register URL");

}

bool login_manager_rvzt::_check_auth_url(const http::url &u)
{
    if (u.host() == "rvzt.zackattackgames.com" ||
        u.host() == "rvzone.zackattackgames.com" ||
        u.host() == "www.revoltrace.com") {
        return true;
    }
    return false;
}
login_manager_rvzt::~login_manager_rvzt() {
    delete _http_fetcher;
}

void
login_manager_rvzt::_login_operation(
  const std::string &req_url,
  const std::string &user,
  int _msg_base
) {
    _fetch_handler *h = new _fetch_handler(this, user, _msg_base);
    http::fetcher::status status = _http_fetcher->fetch(req_url, h);

    if (status != http::fetcher::FETCH_OK) {
        ACE_ERROR((LM_ERROR, "login_manager_rvzt::_login_operation failed (status %d)\n",
                  status));

        gui_messenger()->send_msg(
          new message(LOGIN_MESSAGE(message::login_err)));
        // No need for deleting the handler, seems like ACE calls
        // handle_close() if could not connect which does the deleting
        // delete h;
    }
}

void
login_manager_rvzt::login(const message *msg) {
    const message_login *m = dynamic_ptr_cast<const message_login>(msg);

    std::string req_url = _base_validate_url;
    req_url += "?user=" + m->user();
    req_url += "&pass=" + m->pass();

    _login_operation(req_url, m->user(), message::login);
}

void
login_manager_rvzt::register_user(const message *msg) {
    const message_register *m = dynamic_ptr_cast<const message_register>(msg);

    std::string req_url = _base_register_url;
    req_url += "?user=" + m->user();
    req_url += "&pass=" + m->pass();
    req_url += "&mail=" + m->mail();

    _login_operation(req_url, m->user(), message::reg);
}

// http::handler interface
int
login_manager_rvzt::_fetch_handler::handle_response(const http::response &resp)
{
    ACE_DEBUG((LM_DEBUG, "login_manager_rvzt: got response: %s\n",
              resp.content()));
    const char *str = resp.content();
    std::vector<std::string> res(2);

    if (regexp::match("^([^:]{3,20}):(\\w*)", str, res.begin())) {
    // if (regexp::match("^([a-zA-Z0-9_]{3,20}):(.*)", str, res.begin())) {
        const std::string &user = res[0];
        const std::string &stat = res[1];

        bool validated = false;

        ACE_DEBUG((LM_DEBUG, "login_manager: got user '%s' status '%s'\n",
                   user.c_str(), stat.c_str()));
        if (strcasecmp(user.c_str(), _user.c_str())) {
            ACE_DEBUG((LM_DEBUG, "login_manager: server returned differing " \
                      "username: %s, expected %s\n", user.c_str(),
                      _user.c_str()));
        } else if (stat != "Y") {
            ACE_DEBUG((LM_DEBUG, "login_manager: not validated\n"));
        } else {
            validated = true;
        }

        if (validated) {
            gui_messenger()->send_msg(
              new message_login(LOGIN_MESSAGE(message::login_done),
                                user, ""));
        } else {
            gui_messenger()->send_msg(
              new message_login_fail(LOGIN_MESSAGE(message::login_fail), stat));
        }

    } else {
        ACE_DEBUG((LM_DEBUG, "login_manager_rvzt: could not parse response: %s\n",
        resp.content()));
        gui_messenger()->send_msg(
          new message_login_fail(LOGIN_MESSAGE(message::login_err), "N"));
    }

    return 0;
}

int
login_manager_rvzt::_fetch_handler::handle_error(int reason, const char *details) {
    ACE_DEBUG((LM_DEBUG, "login_manager_rvzt: error: %s\n", details));
    gui_messenger()->send_msg(
      new message(LOGIN_MESSAGE(message::login_err)));
    return 0;
}

int
login_manager_rvzt::_fetch_handler::handle_close() {
    ACE_DEBUG((LM_DEBUG, "login_manager_rvzt: destroying handler\n"));
    delete this;
    ACE_DEBUG((LM_DEBUG, "login_manager_rvzt: destroying handler, done\n"));
    return 0;
}


} // ns networking
