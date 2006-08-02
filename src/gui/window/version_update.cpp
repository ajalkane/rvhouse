#include <string>
#include <sstream>

#include <fx.h>

#include "../../app_version.h"
#include "../window/house_message_box.h"
#include "../house_app.h"
#include "version_update.h"

namespace gui {
namespace window {

version_update::version_update(::message_version *m)
        : _msg(dynamic_cast<message_version *>(m->duplicate()))
{
    ACE_DEBUG((LM_DEBUG, "version_update ctor\n"));
}

version_update::~version_update() {
    ACE_DEBUG((LM_DEBUG, "version_update ctor\n"));
    delete _msg;
}

void
version_update::display() {
    message_version *m = _msg;

    ACE_DEBUG((LM_DEBUG, "version_update::display\n"));

    std::string topic = langstr("new_version_win/title");
    std::ostringstream msgstr;
    if (m->id() == message::version_force) {
        msgstr << langstr("new_version_win/force_msg")
        << std::endl
        << std::endl;
    }
    msgstr << langstr("new_version_win/msg", m->current().c_str(), APP_VERSION)
    << std::endl;
    if (m->whats_new_begin() != m->whats_new_end()) {
        msgstr << langstr("new_version_win/new_since") << std::endl;
        message_version::list_type::const_iterator i = m->whats_new_begin();
        for (; i != m->whats_new_end(); i++) {
            msgstr << "- " << *i << std::endl;
        }
    }
    msgstr << std::endl
    << langstr("new_version_win/download") << std::endl;

    ACE_DEBUG((LM_DEBUG, "version_update::showing box\n"));
    house_message_box::information(::app(), FX::MBOX_OK, m->current_url().c_str(),
                                   topic.c_str(), msgstr.str().c_str());
    ACE_DEBUG((LM_DEBUG, "version_update::showing box quit\n"));

    if (m->id() == message::version_force) {
        ACE_DEBUG((LM_DEBUG, "version_update::was quit\n"));
        ::app()->handle(this, FXSEL(SEL_COMMAND, house_app::ID_QUIT), NULL);
    }
    ACE_DEBUG((LM_DEBUG, "version_update::display() exit\n"));

}

} // ns window
} // ns gui
