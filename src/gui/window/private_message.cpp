//#include <unistd.h>
#include <utility>

#include <QtGui>

#include "../../messaging/message_user.h"
#include "../../messaging/message_room.h"
#include "../../messaging/message_channel.h"
#include "../../messaging/message_send_room.h"
#include "../../messaging/message_room_command.h"
#include "../../model/house.h"
#include "../../model/self.h"
#include "../../icon_store.h"
#include "../house_app.h"
#include "../util/util.h"
#include "private_message.h"

namespace gui {
namespace window {

// Static variable, as must be shared between private_message instances.
// IMPROVE Qt. Probably needs to be moved to rv_house class
// util::bit_slots<16> private_message::_slots;

private_message::private_message(const std::string &id, QWidget *parent)
    // IMPROVE Qt slot of private_message?
    : size_restoring_window<QMainWindow>("private_message", parent),
      _chat_view(NULL), _users_view(NULL), _channel(util::private_message_channel_with(id)), _user_id_str(id)
{
    ACE_DEBUG((LM_DEBUG, "private_message::ctor\n"));
    this->setWindowIcon(app_icons()->get("private_message"));
    this->setContextMenuPolicy(Qt::NoContextMenu);

    _create_actions();
    _create_widgets();
    _create_toolbars();
    _create_layout();

    _connect_signals();

    _init();
}

void
private_message::_create_actions() {
    _action_quit     = new QAction(app_icons()->get("close"), langstr("private_message_win/close"), this);

}

void
private_message::_create_toolbars() {
    QToolBar *tool_bar = addToolBar("room_controls");
    tool_bar->setObjectName("private_message_toolbarclose");
    tool_bar->addAction(_action_quit);
}

void
private_message::_create_widgets() {
    _chat_view  = new view::chat(this, _channel);
    _users_view = new view::users(this);
    _msg_field  = new QLineEdit(this);
    _msg_field->setMaxLength((int)app_opts.limit_chat_msg());

    // This ensures message field keeps the focus even if
    // user scrolls or otherwise operates the other widgets.
    // Is there a better way to do this?
    _chat_view->setFocusProxy(_msg_field);
    _users_view->setFocusProxy(_msg_field);
}

void
private_message::_connect_signals() {
    connect(_msg_field,       SIGNAL(returnPressed()), this, SLOT(send_message()));
    connect(_action_quit,     SIGNAL(triggered()),     this, SLOT(close()));
}

void
private_message::_create_layout() {
    QVBoxLayout *l = new QVBoxLayout;
    // This is used in _create_toolbars

    _chat_users_splitter = new QSplitter(Qt::Horizontal);
    QSplitter *vsplitter = new QSplitter(Qt::Vertical);
    vsplitter->addWidget(_chat_view);
    _chat_users_splitter->addWidget(vsplitter);
    _chat_users_splitter->addWidget(_users_view);

    l->addWidget(_chat_users_splitter);
    l->addWidget(_msg_field);

    vsplitter->setStretchFactor(1, -1);
    _chat_users_splitter->setStretchFactor(0, -1);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(l);
    this->setCentralWidget(centralWidget);

    _msg_field->setFocus();
}

void
private_message::show() {
    super::show();

    QList<int> _chat_users_splitter_sizes = _chat_users_splitter->sizes();
    _chat_users_splitter_sizes.back() = 150;
    _chat_users_splitter->setSizes(_chat_users_splitter_sizes);
}

void
private_message::_init() {
    model::house::user_iterator ui = house_model()->user_find(_user_id_str);
    if (ui != house_model()->user_end())
        this->setWindowTitle(ui->display_id().c_str());

    _users_view->observer_set(this);
}

private_message::~private_message() {
    ACE_DEBUG((LM_DEBUG, "private_message::dtor\n"));
}

void
private_message::send_message() {
    QString t = _msg_field->text();

    if (t.isEmpty()) return;

    if (!_flood_control.allow_send()) {
        _chat_view->status_message(langstr("chat/flood_control"));
        return;
    }

    if (t.length() > (int)app_opts.limit_chat_msg())
        t.truncate(app_opts.limit_chat_msg());
    // IMPROVE Qt is this needed?
    // t.substitute("\r", "");

    // Sending a room message
    message_channel *msg = 
      new message_channel(::message::send_private,
                          t.toLatin1().constData(),
                          _user_id_str,
                          _channel,
                          self_model()->sequence(),
                          0);
    net_messenger()->send_msg(msg);

    _msg_field->setText("");
}

void
private_message::handle_message(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "private_message::handle_message\n"));
    
    _chat_view->handle_message(msg);

    message_grouped *mg = dynamic_cast<message_grouped *>(msg);
    if (!mg) return;
    
    switch (msg->id()) {
    case ::message::user:
    case ::message::user_left:
    {
        // Only pass user messages to users view about the users
        // participating in the private chat
        message_user *u = dynamic_ptr_cast<message_user>(msg);
        if (u->user().id().id_str() == _user_id_str ||
            u->user().id().id_str() == self_model()->user().id().id_str()) 
        {
            _users_view->handle_message(msg);
        }
    }
        break;
    default:
        return;
    }   
}

void
private_message::user_added(const chat_gaming::user &u) {
    _chat_view->status_message(
        langstr("chat/user_joined_room", u.display_id().c_str())
    );
}
void
private_message::user_removed(const chat_gaming::user &u) {
    _chat_view->status_message(
        langstr("chat/user_exited_room", u.display_id().c_str())
    );
}

} // ns window
} // ns gui
