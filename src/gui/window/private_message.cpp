#include <unistd.h>
#include <utility>

#include <fx.h>
#include <fxkeys.h>

#include "../../messaging/message_user.h"
#include "../../messaging/message_room.h"
#include "../../messaging/message_channel.h"
#include "../../messaging/message_send_room.h"
#include "../../messaging/message_room_command.h"
#include "../../util.h"
#include "../../model/house.h"
#include "../../model/self.h"
#include "../../executable/launcher.h"
#include "../../icon_store.h"
#include "../house_app.h"
#include "../util/util.h"
#include "private_message.h"

namespace gui {
namespace window {

// Static variable, as must be shared between private_message instances.
util::slots<16> private_message::_slots;

FXDEFMAP(private_message) private_message_map[]= {
  FXMAPFUNC(SEL_COMMAND,  private_message::ID_SEND_MSG, 
                          private_message::on_send_message),
};

// FXIMPLEMENT(private_message, FXMainWindow, private_message_map, ARRAYNUMBER(private_message_map))
FXIMPLEMENT(private_message, private_message::super, private_message_map, ARRAYNUMBER(private_message_map))

// FXIMPLEMENT(private_message, FXMainWindow, NULL, 0)

private_message::private_message(FXApp *a, const std::string &id)
    : super(a, "", NULL, NULL, DECOR_ALL, 0, 0, 350, 300), // , 0,0,0,0,0,0),
      _chat_view(NULL), _users_view(NULL), _user_id_str(id)
    // : FXMainWindow(a, "", NULL, NULL, DECOR_ALL, 0, 0, 350, 300),
{
    _init2();
}

/*
private_message::private_message(FXWindow *owner, const std::string &id)
    : super(owner, "",  NULL, NULL, DECOR_ALL, 0, 0, 350, 300, 0,0,0,0,0,0),
      _user_id_str(id)
    // : FXMainWindow(a, "", NULL, NULL, DECOR_ALL, 0, 0, 350, 300),
{
    _init();
}
*/

void
private_message::_init2() {
    ACE_DEBUG((LM_DEBUG, "private_message::ctor %d\n", (FXWindow *)this));

    setIcon(app_icons()->get("private_message"));
    setMiniIcon(app_icons()->get("private_message"));

    model::house::user_iterator ui = house_model()->user_find(_user_id_str);
    if (ui != house_model()->user_end())
        setTitle(ui->display_id().c_str());

    int button_opts = ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED;

    FXVerticalFrame *c = new FXVerticalFrame(
        this, LAYOUT_FILL_X|LAYOUT_FILL_Y,
        0,0,0,0, 0,0,0,0, 0,0
    );    
    FXComposite *toolbarcontainer = new FXHorizontalFrame(
        c, LAYOUT_SIDE_TOP|LAYOUT_FILL_X,
        0,0,0,0, 0,0,0,0, 0,0
    );
    new FXToolBarTab(toolbarcontainer,NULL,0,FRAME_RAISED);
    FXComposite *toolbar = new FXToolBar(
        toolbarcontainer,
        FRAME_RAISED|
        LAYOUT_SIDE_TOP|LAYOUT_FILL_X,
        0,0,0,0, 4,4,4,4, 0,0
    );

    new FXButton(toolbar, 
                 util::button_text(NULL, langstr("private_message_win/close")),
                 app_icons()->get("close"), 
                 this, ID_CLOSE, button_opts);

    // new FXVerticalSeparator(toolbar);

    FXSplitter *house     = new FXSplitter(c, SPLITTER_HORIZONTAL |
                                                 SPLITTER_REVERSED   |
                                                 LAYOUT_FILL_X |
                                                 LAYOUT_FILL_Y);
    FXSplitter *sections  = new FXSplitter(house, LAYOUT_FILL_X | 
                                                  LAYOUT_FILL_Y |
                                                  SPLITTER_VERTICAL);

    _users_view = new view::users(util::framed_container(house), NULL); // , 0, FRAME_SUNKEN|FRAME_THICK);

    _chat_view = new view::chat(util::framed_container(sections));

    // _chat_view->channel(_room_id);
    // FXFrame *f = new FXFrame(this);
    FXVerticalFrame *b = new FXVerticalFrame(
        c, LAYOUT_FILL_X,
        0,0,0,0,
        0,0);

    _msg_field = new FXTextField(b, 0, this, ID_SEND_MSG, 
                                 FRAME_SUNKEN|FRAME_THICK|
                                 LAYOUT_FILL_X|TEXTFIELD_ENTER_ONLY);

    _msg_field->setFocus(); 
    
    house->setSplit(1, 150);

    // Find a free slot for this private message window
    _slot = _slots.reserve();
    if (_slot.valid()) {
        std::string win_name = "private_message" + _slot.as_str();
        // For some reason, restore_size has to be in constructor
        // if FXMainWindow and in create otherwise
        util::restore_size(this, win_name.c_str());
    }

    // _users_view->setWidth(150);
    _users_view->observer_set(this);
    
    getAccelTable()->addAccel(MKUINT(KEY_F4,ALTMASK),this,FXSEL(SEL_COMMAND,ID_CLOSE));

    _channel = util::private_message_channel_with(_user_id_str);
    _chat_view->channel(_channel);
    
    ACE_DEBUG((LM_DEBUG, "private_message::ctor channel %s\n",
              _channel.c_str()));   
    ACE_DEBUG((LM_DEBUG, "private_message::ctor done\n"));
}

private_message::~private_message() {
    ACE_DEBUG((LM_DEBUG, "private_message: dtor\n"));

    if (_slot.valid()) {
        std::string win_name = "private_message" + _slot.as_str();
        // For some reason, restore_size has to be in constructor
        // if FXMainWindow and in create if FXDialogBox.
        util::store_size(this, win_name.c_str());

        _slots.free(_slot);
    }
    
    delete _users_view;
    delete _chat_view;
    
    ACE_DEBUG((LM_DEBUG, "private_message: dtor done\n"));  
}

void
private_message::create() {
    super::create();
    watched_window::create(this);

    // show(PLACEMENT_SCREEN);
    show();
}

long 
private_message::on_send_message(FXObject *from, FXSelector sel, void *) {
    FXString t = _msg_field->getText();

    if (t.empty()) return 1;
    
    if (!_flood_control.allow_send(t)) {
        _chat_view->status_message(langstr("chat/flood_control"));
        return 0;        
    }
    
    if (t.length() > (int)app_opts.limit_chat_msg()) 
        t.trunc(app_opts.limit_chat_msg());
    t.substitute("\r", "");

    // TODO away
    // ACE_OS::sleep(5);

    // Sending a private message
    message_channel *msg = 
      new message_channel(::message::send_private,
                          t.text(),
                          _user_id_str,
                          _channel,
                          self_model()->sequence(),
                          0);
    net_messenger()->send_msg(msg);
        
    _msg_field->setText("");
    
    return 1;
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
