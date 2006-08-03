#include "../../main.h"
#include "../../app_options.h"
#include "../../executable/launcher.h"
#include "../house_app.h"

#include "router_fw_help.h"

namespace gui {
namespace window {
    
void
router_fw_help::display() {
    FXMessageBox::error(::app(), FX::MBOX_OK, 
                        langstr("app/dht_connect_err_title"),
                        langstr_range("app/dht_connect_err",1,10,"\n").c_str());

    ACE_DEBUG((LM_DEBUG, "router_fw_help::launching url %s\n",
              ROUTER_FW_HELP_URL)); 
    launcher_file()->start(ROUTER_FW_HELP_URL);
}

} // ns window
} // ns gui
