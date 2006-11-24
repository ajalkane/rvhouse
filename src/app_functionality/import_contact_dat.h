#ifndef _APP_FUNCTIONALITY_IMPORT_CONTACT_DAT_H_
#define _APP_FUNCTIONALITY_IMPORT_CONTACT_DAT_H_

#include <string>

#include "base.h"

namespace app_functionality {

class import_contact_dat : public base {
    std::string _import_from;

public:
    import_contact_dat(int argc, char **argv,
                       const std::string &file_name);
    virtual ~import_contact_dat();
    
    virtual void run();
};

} // ns app_functionality

#endif // _APP_FUNCTIONALITY_IMPORT_CONTACT_DAT_H_
