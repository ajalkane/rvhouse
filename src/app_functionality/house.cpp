#include <ace/ACE.h>

#include <QtGui>

#include "../common.h"
#include "../gui/house_app.h"
#include "../gui/window/house.h"
#include "house.h"

namespace app_functionality {

house::house(int &argc, char **argv) {
    gui::house_app *house_app = new gui::house_app(argc, argv);
    app.instance(house_app);
}

void
house::run() {
    if (app()->start()) {
        app()->exec();
    }

    delete app();
    app.instance(NULL);
}

} // ns
