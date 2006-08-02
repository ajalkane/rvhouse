#include "launcher.h"

namespace executable {

launcher::launcher() {}
launcher::launcher(const std::string &default_app)
        : _default_app(default_app) {}

launcher::~launcher() {}

int
launcher::start() {
    return (_default_app.empty()
            ? err_not_supported
            : start(_default_app));
}

int
launcher::start_host() {
    return (_default_app.empty()
            ? err_not_supported
            : start_host(_default_app));
}

int
launcher::start_client(const std::string &host_id) {
    return (_default_app.empty()
            ? err_not_supported
            : start_client(_default_app, host_id));
}


} // ns executable
