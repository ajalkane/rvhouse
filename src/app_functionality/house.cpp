#include <fx.h>

#include "../common.h"
#include "../gui/house_app.h"
#include "house.h"

namespace app_functionality {

house::house(int argc, char **argv) {
    ACE_DEBUG((LM_DEBUG, "init_fox_app\n"));
    app.instance(new gui::house_app);
    app()->init(argc, argv);
    new FXToolTip(app());
}

void
house::run() {
    app()->create();
    app()->start();
}

} // ns
