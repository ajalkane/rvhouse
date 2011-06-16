#ifndef _QT_UTIL_H_
#define _QT_UTIL_H_

#include <QSettings>

// Be aware that you will have to destroy the QSettings object later!
QSettings* copy_qsettings(const QSettings &from);

#endif //_QT_UTIL_H_
