#include <algorithm>

#include "messenger.h"

#include "../util.h"

using namespace std;

messenger::messenger()
    : _lock(true)
{
}

messenger::~messenger() {
    for_each(_msgs.begin(), _msgs.end(), delete_ptr<message>());
}
