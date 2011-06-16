#include <string.h>

#include "regexp.h"

namespace regexp {

bool match(const char *pattern, const char *subject) {
    QRegExp rex(pattern);

    return rex.indexIn(subject) > -1;
}

}
