#ifndef _GUI_UTIL_SLOTS_
#define _GUI_UTIL_SLOTS_

#include <string>
#include <sstream>
#include <bitset>

#include "../../common.h"

namespace gui {
namespace util {

class slot {
public:
    inline slot(int n) : _n(n)  {}
    inline slot()      : _n(-1) {}
    inline int         as_int() const { return _n; }
    inline std::string as_str() const {
        std::ostringstream nstr;
        nstr << _n;
        return nstr.str();  
    }
    inline bool valid() const { return _n > 0; }
    inline bool invalid() const { return !valid(); }
private:
    int _n;
};

template <unsigned int N>
class bit_slots {
public:
    typedef std::bitset<N> container_type;
    
    inline slot reserve() {
        
        for (unsigned int n = 0; n < _slots.size(); n++) {
            if (!_slots.test(n)) {
                _slots.set(n, 1);
                return slot(n + 1);
            }
        }
        return slot(0);
    }
    inline void free(const slot &s) {
        if (s.valid()) {
            _slots.set(s.as_int() - 1, 0);
        }
    }
    
private:
    container_type _slots;
};

} // ns util
} // ns gui

#endif // _GUI_UTIL_SLOTS_H_
