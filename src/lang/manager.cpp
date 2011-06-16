#include <utility>
#include <map>

#include <QtGui>

#include "../config_file.h"
#include "../exception.h"

#include "manager.h"

namespace lang {

manager::manager() {
}

manager::~manager() {
}
    
void
manager::init(const std::string &dir) {
    QDir qdir(dir.c_str());
    QFileInfoList file_infos = qdir.entryInfoList(QStringList("*.def"));
    QFileInfoList::const_iterator fi = file_infos.begin();
    for (; fi != file_infos.end(); fi++) {
        QFileInfo file_info = *fi;
        ACE_DEBUG((LM_DEBUG, "lang::manager::init def file %s\n",
                  file_info.filePath().toLatin1().constData()));
        std::string path_noext(qdir.filePath(file_info.completeBaseName()).toLatin1().constData());
        config_file df(config_file::ini_file);
        df.load(path_noext + ".def");

        std::list<info>::iterator i = _infos.insert(_infos.end(), info());
        i->_lang    = df.get("info/language");
        i->_author  = df.get("info/author");
        i->_email   = df.get("info/email");
        i->_version = df.get<std::string>("info/version", "0.81");
        i->_file    = path_noext + ".lang";

        ACE_DEBUG((LM_DEBUG, "lang::manager::init lang definition: ",
                  "%s by %s in file %s\n",
                  i->lang().c_str(), i->author().c_str(), i->file().c_str()));
    }

    if (_infos.size() == 0)
        throw exceptionf(0, "Language files not found. Reinstall RV House");
}

void
manager::mapper_init(mapper &m, const std::string &name, const std::string &def) {
    _mapper_fill(m, name);
    if (!def.empty() && def != name) {
        m.set_mode(mapper::set_default);
        _mapper_fill(m, def);
    }
}

void
manager::_mapper_fill(mapper &m, const std::string &name) {
    ACE_DEBUG((LM_DEBUG, "lang::manager: filling mapper for %s\n", name.c_str()));
    
    // First find the matching file
    std::list<info>::const_iterator i = _infos.begin();
    for (; i != _infos.end(); i++) {
        if (i->lang() == name) break;
    }
    
    if (i == _infos.end())
        throw exceptionf(0, "No language file found for %s. "
                         "Reinstall RV House", name.c_str());

    ACE_DEBUG((LM_DEBUG, "lang::manager: parsing file %s\n", i->file().c_str()));
    config_file lf(config_file::ini_file);
    lf.load(i->file());
    
    config_file::key_value_map_type key_value_map;
    lf.keys_and_values_as_map(key_value_map);
    config_file::key_value_map_type::iterator kv_i;
    for (kv_i = key_value_map.begin(); kv_i != key_value_map.end(); ++kv_i) {
        m.set(kv_i->first.c_str(), kv_i->second.c_str());
    }
}

} // ns lang
