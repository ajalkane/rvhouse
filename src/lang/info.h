#ifndef _LANG_INFO_
#define _LANG_INFO_

#include <string>

namespace lang {

// Forward declaration
class manager;

class info {
    std::string _lang;
    std::string _author;
    std::string _email;
    std::string _file;
    std::string _version;
    
    friend class manager;   
public:
    inline const std::string &lang()    const { return _lang;  }
    inline const std::string &author()  const { return _author;  }
    inline const std::string &email()   const { return _email; }
    inline const std::string &file()    const { return _file;  }
    inline const std::string &version() const { return _version; }
};
    
} // ns lang

#endif //_LANG_INFO_
