#ifndef REGEXP_H
#define REGEXP_H

#include <string>

#include <QString>
#include <QStringList>
#include <QRegExp>

#include "common.h"

namespace regexp {
    bool match(
        const char *pattern, 
        const char *subject
    );
    inline bool match(
        const std::string &pattern, 
        const std::string &subject
    ) {
        return match(pattern.c_str(), subject.c_str());
    }
        
    
    // Match case insensitive verbatim (without magic chars)
    inline bool matchiv(const std::string &p, const std::string &s) {
        QString pattern(p.c_str());
        return pattern.contains(s.c_str());
    }
    
    // Use this if there is need to capture parenthesed expressions,
    template <typename OutIter>
    bool match(const std::string &pattern, const std::string &subject,
               OutIter results)
    {
        QRegExp rex(pattern.c_str());
        
        if (rex.indexIn(subject.c_str()) > -1) {
            QStringList caps = rex.capturedTexts();
            for (int i = 1; i < caps.size(); ++i) {
                QString cap = caps.at(i);
                *results++ = std::string(cap.toLatin1().constData());
            }
            return true;
        }
        return false;
    }
}

#endif //REGEXP_H
