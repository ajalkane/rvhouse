#include "config_file.h"
#include "fx_util.h"
#include "exception.h"

// no_space_as_ws_ctype config_file::_loc_facet = new no_space_as_ws_ctype();
std::locale          config_file::_loc =
  std::locale(std::locale(), new no_space_as_ws_ctype());

config_file::config_file() :
    _conf(new FXSettings),
    _no_delete(false)
{}

config_file::config_file(const config_file &o) {
    FXSettings *from_conf = const_cast<FXSettings *>(o._conf);
    _conf = CopyFXSettings(from_conf),
    _no_delete = false;
}

config_file::config_file(const std::string &app, const std::string &vendor)
    : _conf(new FXRegistry(app.c_str(), vendor.c_str())),
      _no_delete(false)
{
    ACE_DEBUG((LM_DEBUG, "config_file::ctor registry vendor/app: %s/%s\n",
              vendor.c_str(), app.c_str()));
    // FXRegistry *rconf = dynamic_cast<FXRegistry *>(_conf);
    // rconf->read();
}

config_file::config_file(FXRegistry &reg)
    : _conf(&reg),
      _no_delete(true)
{
}

config_file::~config_file() {
    if (!_no_delete)
        delete _conf;
}

void
config_file::load(const std::string &file) {
    if (!load_conditionally(file))
        throw exceptionf(0, "Could not find configuration file %s",
                         file.c_str());

    _parsed_file = file;
}

bool
config_file::load_conditionally(const std::string &file) {
    if (!_conf->parseFile(file.c_str(), true)) return false;
    _parsed_file = file;
    return true;
}

void
config_file::load() {
    FXRegistry *rconf = dynamic_cast<FXRegistry *>(_conf);
    if (!rconf)
        throw exceptionf(0, "config_file called but no source defined/inferred");

    rconf->read();
}

void
config_file::save(const std::string &file) {
    ACE_DEBUG((LM_DEBUG, "config_file: writing file %s\n",
              file.c_str()));

    if (!_conf->unparseFile(file.c_str()))
        throw exceptionf(0, "Could not save configuration to %s",
                         file.c_str());
}

void
config_file::save() {
    if (_parsed_file.empty()) {
        FXRegistry *rconf = dynamic_cast<FXRegistry *>(_conf);
        if (rconf == NULL)
            throw exceptionf(0, "No configuration file parsed, can not save");
        rconf->write();
        return;
    }
    save(_parsed_file);
}

config_file &
config_file::operator=(const config_file &o) {
    // First delete old configuartion, and then create a copy of the
    // other
    delete _conf;
    FXSettings *from_conf = const_cast<FXSettings *>(o._conf);
    _conf = CopyFXSettings(from_conf);
    return *this;
}

size_t
config_file::sections_as_list(std::list<std::string> &l) {
    size_t c = 0;
    for (FXint i = _conf->first(); i < _conf->size(); i = _conf->next(i)) {
        l.push_back(_conf->key(i));
        c++;
    }
    return c;
}

size_t
config_file::section_as_map(const char *section, std::map<std::string,std::string> &m) {
    FXStringDict *sec = _conf->find(section);
    if (!sec) return 0;

    size_t c = 0;
    for (FXint i = sec->first(); i < sec->size(); i = sec->next(i)) {
        m[sec->key(i)] = sec->data(i);
        c++;
    }
    return c;
}
