#include <utility>
#include <map>

#include <fx.h>

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
    FXString *filelist = NULL;
    FXint     files 
      = FXFile::listFiles(filelist, dir.c_str(), "*.def");
      
    ACE_DEBUG((LM_DEBUG, "lang::manager::init %d language files found\n", files));
    for (int file_ndx = 0; file_ndx < files; file_ndx++) {
        ACE_DEBUG((LM_DEBUG, "lang::manager::init def file %s\n",
                  filelist[file_ndx].text()));
        FXString file_noext = FXFile::stripExtension(filelist[file_ndx]);

        // Create the full file name path
        std::string path_noext(dir);
        path_noext += PATHSEP;
        path_noext += file_noext.text();

        // Parse each language definition file and create info class for it
        config_file df;
        df.parse(path_noext + ".def");
        
        std::list<info>::iterator i = _infos.insert(_infos.end(), info());
        i->_lang    = df.get("info", "language");
        i->_author  = df.get("info", "author");
        i->_email   = df.get("info", "email");
        i->_version = df.get<std::string>("info", "version", "0.81");
        i->_file   = path_noext + ".lang";

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
    config_file lf;
    lf.parse(i->file());
    
    std::list<std::string> sections;
    std::list<std::string>::const_iterator sec_i;
    ACE_DEBUG((LM_DEBUG, "lang::manager: grabbing sections as list\n"));
    lf.sections_as_list(sections);

    for (sec_i = sections.begin(); sec_i != sections.end(); sec_i++) {
        std::map<std::string, std::string> values;
        std::map<std::string, std::string>::const_iterator val_i;
        ACE_DEBUG((LM_DEBUG, "lang::manager: grabbing section as map\n"));
        lf.section_as_map(sec_i->c_str(), values);
        for (val_i = values.begin(); val_i != values.end(); val_i++) {
            std::string key;
            key = *sec_i + "/" + val_i->first;
            ACE_DEBUG((LM_DEBUG, "lang::manager: setting\n"));
            m.set(key.c_str(), val_i->second.c_str());
        }
    }
}

} // ns lang
