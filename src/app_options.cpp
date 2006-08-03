#include "main.h"
#include "app_options.h"
#include "config_file.h"

app_options::app_options()
  : _debug(false)
{
}

void
app_options::init() {
    _debug = conf()->get<bool>("main", "debug", false);
}
