#include <QtGui>

#include "../../main.h"
#include "../../app_options.h"
 #include "../../executable/launcher.h"

#include "router_fw_help.h"

namespace gui {
namespace window {
    
router_fw_help::router_fw_help(QWidget *parent) : modal_dialog_guard(parent) {
    this->setWindowTitle(langstr("app/dht_connect_err_title"));

    QVBoxLayout *l = new QVBoxLayout;
    l->setSpacing(0);

    l->addWidget(new QLabel(langstr_range("app/dht_connect_err",1,9,"\n").c_str()));

    /* Button part and layout */
    l->addStretch(1);
    QDialogButtonBox *button_box = new QDialogButtonBox;
    QPushButton *ok_button = new QPushButton(langstr("common/ok_button"), this);
    button_box->addButton(ok_button, QDialogButtonBox::AcceptRole);
    l->addWidget(button_box);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));

    this->setLayout(l);

    ACE_DEBUG((LM_DEBUG, "router_fw_help::ctor() exit\n"));
}

router_fw_help::~router_fw_help() {
    ACE_DEBUG((LM_DEBUG, "router_fw_help::launching url %s\n",
              ROUTER_FW_HELP_URL));
    launcher_file()->start(ROUTER_FW_HELP_URL);
}

} // ns window
} // ns gui
