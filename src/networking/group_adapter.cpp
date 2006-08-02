#include "group_adapter.h"

namespace networking {

group_adapter::group_adapter(
    ACE_Reactor *r
) : _reactor(r)
{
}

group_adapter::~group_adapter() {
}

void
group_adapter::init() {}

} // ns networking

