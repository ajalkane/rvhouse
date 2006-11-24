#ifndef _APP_FUNCTIONALITY_EXPORT_CONTACT_DAT_H_
#define _APP_FUNCTIONALITY_EXPORT_CONTACT_DAT_H_

#include <string>

#include "base.h"

namespace app_functionality {

class export_contact_dat : public base {
    std::string _export_to;

public:
    export_contact_dat(int argc, char **argv,
                       const std::string &file_name);
    virtual ~export_contact_dat();
    
    virtual void run();
};

} // ns app_functionality

#endif // _APP_FUNCTIONALITY_EXPORT_CONTACT_DAT_H_
