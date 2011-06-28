#include <QtGui>

#include "config_file.h"
#include "qt_util.h"
#include "exception.h"
#include "app_version.h"

config_file::config_file(config_file::config_type type) :
    _no_delete(false)
{
    switch (type) {
    case ini_file:
        _conf = NULL;
        break;
    case registry:
        _conf = new QSettings(APP_SETTINGS_ORG_KEY, APP_SETTINGS_APP_KEY);
        break;
    }
}

config_file::~config_file() {
    if (!_no_delete)
        delete _conf;
}

void
config_file::load(const std::string &file) {
    switch(load_conditionally(file)) {
    case access_error:
        throw exceptionf(0, "Error accessing configuration file %s",
                         file.c_str());
        break;
    case format_error:
        throw exceptionf(0, "Format error in configuration file %s",
                         file.c_str());
        break;
    case does_not_exist_or_is_empty:
        throw exceptionf(0, "Configuration file not found or is empty at %s",
                         file.c_str());
        break;
    case other_error:
        throw exceptionf(0, "Error loading configuration file %s",
                         file.c_str());
        break;
    case no_error:
        return;
    }
}

config_file::load_status _qsettings_status_to_load_status(QSettings::Status qstatus) {
    switch (qstatus) {
    case QSettings::NoError:
        return config_file::no_error;
    case QSettings::AccessError:
        return config_file::access_error;
    case QSettings::FormatError:
        return config_file::format_error;
    }
    return config_file::other_error;
}

config_file::load_status
config_file::load_conditionally(const std::string &file) {
    _conf = new QSettings(file.c_str(), QSettings::IniFormat);
    QStringList keys = _conf->allKeys();
    ACE_DEBUG((LM_DEBUG, "config_file::load_conditionally: file '%s', qtstatus '%d', size %d\n",
               _conf->fileName().toLatin1().constData(), _conf->status(), keys.size()));
    if (_conf->status() != QSettings::NoError) return _qsettings_status_to_load_status(_conf->status());
    if (_conf->allKeys().isEmpty()) return does_not_exist_or_is_empty;
    return no_error;
}

std::string
config_file::get_value(const char *key) const {
    QVariant v_value = _conf->value(key, "");
    return std::string(v_value.toString().toLatin1().constData());
}

bool
config_file::contains(const char *key) const {
    return _conf->contains(key);
}

void
config_file::set_value(const char *key, const char *value) {
    _conf->setValue(key, QVariant(value));
}

void
config_file::save() {
    _conf->sync();
}


size_t
config_file::keys_and_values_as_map(key_value_map_type &m) const {
    QStringList keys = _conf->allKeys();
    QStringList::const_iterator i = keys.begin();
    size_t items = 0;
    for (; i != keys.end(); ++i, ++items) {
        QString key = *i;
        QVariant value = _conf->value(key);
        m[key.toLatin1().constData()] = value.toString().toLatin1().constData();
    }

    return items;
}
