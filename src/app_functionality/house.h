#ifndef _APP_FUNCTIONALITY_HOUSE_H_
#define _APP_FUNCTIONALITY_HOUSE_H_

#include "base.h"

namespace app_functionality {

class house : public base {
public:
    house(int argc, char **argv);
    virtual void run();
};

} // ns app_functionality

#endif // _APP_FUNCTIONALITY_HOUSE_H_
