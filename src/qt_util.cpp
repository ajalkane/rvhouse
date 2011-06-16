#include <ace/Log_Msg.h>
#include <QtGui>

#include "qt_util.h"

QSettings *copy_qsettings(const QSettings &from)
{
    ACE_DEBUG((LM_DEBUG, "copy_qsettings: format %d, scope %d, organizationName %s, applicationName %s\n",
            from.format(), from.scope(), from.organizationName().toLatin1().constData(), from.applicationName().toLatin1().constData()));

    QSettings *copy = new QSettings(); // from.format(), from.scope(), from.organizationName(), from.applicationName());
    ACE_DEBUG((LM_DEBUG, "copy_qsettings: new QSettings size %d\n", copy->allKeys().size()));

    QStringList keys = from.allKeys();
    QStringList::const_iterator i = keys.begin();
    for (; i != keys.end(); i++) {
        QString key = *i;
        QVariant value = from.value(key);
        copy->setValue(key, value);
        ACE_DEBUG((LM_DEBUG, "copy_qsettings: key '%s', value '%s'\n",
                    key.toLatin1().constData(), value.toString().toLatin1().constData()));
    }
    
    // Return copied settings
    return copy;
}
