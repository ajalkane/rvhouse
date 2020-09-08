#include <string>
#include <sstream>

#include <QtGui>

#include "../../app_version.h"
#include "../house_app.h"
#include "version_update.h"

namespace gui {
namespace window {

version_update::version_update(::message_version *m, QWidget *parent)
    : modal_dialog_guard(parent), _msg(dynamic_cast<message_version *>(m->duplicate()))
{
    ACE_DEBUG((LM_DEBUG, "version_update ctor\n"));

    this->setWindowTitle(langstr("new_version_win/title"));
    
    ACE_DEBUG((LM_DEBUG, "version_update::display\n"));

    QVBoxLayout *l = new QVBoxLayout;
    l->setSpacing(0);

    if (m->id() == message::version_force) {
        l->addWidget(new QLabel(langstr("new_version_win/force_msg")));
        l->addWidget(new QLabel);
    }
    l->addWidget(new QLabel(langstr("new_version_win/msg", m->current().c_str(), APP_VERSION).c_str()));

    if (m->whats_new_begin() != m->whats_new_end()) {
        l->addWidget(new QLabel(langstr("new_version_win/new_since")));
        message_version::list_type::const_iterator i = m->whats_new_begin();
        for (; i != m->whats_new_end(); i++) {
            std::string whats_new = "- ";
            whats_new += *i;
            l->addWidget(new QLabel(whats_new.c_str()));
        }
    }
    l->addWidget(new QLabel);
    
    l->addWidget(new QLabel(langstr("new_version_win/download")));

#ifdef WIN32
    std::string suffix = "_setup.exe";
#else
    std::string suffix = "_linux.tar.gz";
#endif
    std::string ahref = "<a href=\"";
    ahref.append(m->current_url());
    ahref.append(suffix);
    ahref.append("\">");
    ahref.append(m->current_url());
    ahref.append(suffix);
    ahref.append("</a>");

    QLabel *label = new QLabel(ahref.c_str());
    label->setOpenExternalLinks(true);
    l->addWidget(label);

    l->addStretch(1);
    QDialogButtonBox *button_box = new QDialogButtonBox;
    QPushButton *ok_button = new QPushButton(langstr("common/ok_button"), this);
    button_box->addButton(ok_button, QDialogButtonBox::AcceptRole);
    l->addWidget(button_box);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));

    this->setLayout(l);

    ACE_DEBUG((LM_DEBUG, "version_update::ctor() exit\n"));

}

version_update::~version_update() {
    ACE_DEBUG((LM_DEBUG, "version_update ctor\n"));
    if (_msg->id() == message::version_force) {
        ACE_DEBUG((LM_DEBUG, "version_update::was quit\n"));
        ::app()->quit();
    }

    delete _msg;
}

} // ns window
} // ns gui
