#include "config.h"

#include "util.h"

// no_space_as_ws_ctype config_file::_loc_facet = new no_space_as_ws_ctype();
std::locale          config::_loc =
  std::locale(std::locale(), new no_space_as_ws_ctype());

config::~config() {

}
