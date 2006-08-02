#ifndef _EXECUTABLE_FACTORY_H_
#define _EXECUTABLE_FACTORY_H_

#include <string>

namespace executable {

class factory {
public:
    class launcher *create_launcher(const std::string &app_id);
};

}
#endif //_EXECUTABLE_FACTORY_H_
