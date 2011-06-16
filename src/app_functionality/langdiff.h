#ifndef _APP_FUNCTIONALITY_LANGDIFF_H_
#define _APP_FUNCTIONALITY_LANGDIFF_H_

#include <string>
#include <sstream>
#include <list>

#include <QApplication>

#include "house.h"

namespace app_functionality {

class langdiff : public base {
    std::string _lang1;
    std::string _lang2;

    std::ostringstream _out;
    
    QApplication *_app;
            
    void _print_uniq(
        const std::list<std::string> &l, 
        const std::string &lang
    );
public:
    langdiff(int &argc, char **argv,
             const std::string &lang1, 
             const std::string &lang2);
    virtual ~langdiff();
    
    virtual void run();
};

} // ns app_functionality

#endif // _APP_FUNCTIONALITY_LANGDIFF_H_
