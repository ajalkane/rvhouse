#ifndef _APP_FUNCTIONALITY_BASE_H_
#define _APP_FUNCTIONALITY_BASE_H_

namespace app_functionality {

class base {
public:
    virtual ~base();
    virtual void run() = 0; 
};

} // ns app_functionality

#endif // _APP_FUNCTIONALITY_BASE_H_
