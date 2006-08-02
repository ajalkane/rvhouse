#include "login_manager.h"

namespace networking {

login_manager::login_manager(
    ACE_Reactor *r
) : _reactor(r)
{
}

login_manager::~login_manager() {
}

} // ns networking

