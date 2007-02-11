            #include <sstream>

#include <fx.h>

#include "../../util.h"
#include "../../regexp.h"
#include "../../os_util.h"
#include "../../main.h"
#include "../../app_options.h"
#include "../../messaging/message_channel.h"
#include "../../messaging/message_room.h"
#include "../../messaging/message_user.h"
#include "../../messaging/message_block_users.h"
#include "../../messaging/message_global_ignore.h"
#include "../../model/house.h"
#include "../../model/self.h"
#include "../../parser/url/dfa.h"
#include "../../executable/launcher.h"

#include "chat.h"

namespace gui {
namespace view {

FXDEFMAP(chat) chat_map[]= {
    FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,chat::on_left_button),
    FXMAPFUNC(SEL_TIMEOUT,chat::ID_TIMER, chat::on_timer),  
    FXMAPFUNC(SEL_MOTION, 0, chat::on_motion),
    FXMAPFUNC(SEL_COMMAND, chat::ID_INPUT, 
                           chat::on_input),
    FXMAPFUNC(SEL_CONFIGURE, chat::ID_CONFIGURE,chat::on_configure)                           
};


FXIMPLEMENT(chat, FXText, chat_map, ARRAYNUMBER(chat_map))
    
namespace {
    inline bool is_word_separator(char c) {
        switch (c) {
        case 0:
        case ' ':
        case '\t': case '\n': case '\r':
        case ')':  case '(':
        case '{':  case '}':
        case '[':  case ']':
        case '.':  case ',': case ':': case ';':
            return true;
        }
        return false;
    }
}
    
chat::chat(
    FXComposite *c, FXObject *tgt, 
    FXSelector sel, FXuint opts, 
    FXint x, FXint y, 
    FXint w, FXint h, 
    FXint pl, FXint pr, 
    FXint pt, FXint pb)
: FXText(c, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb),
  _flash_window(NULL),
  _allow_scroll(true)
{
    
    _current_cursor = DEF_ARROW_CURSOR;
    
    _styles = new FXHiliteStyle[style_last - 1];
    // Set defaults
    for (int i = 0; i < style_last - 1; i++) {
        _styles[i].normalForeColor = getApp()->getForeColor();
        _styles[i].normalBackColor = getApp()->getBackColor();
        _styles[i].selectForeColor = getApp()->getSelforeColor();
        _styles[i].selectBackColor = getApp()->getSelbackColor();
        _styles[i].hiliteForeColor = getApp()->getHiliteColor();
        _styles[i].hiliteBackColor = getHiliteBackColor();
        _styles[i].activeBackColor = getApp()->getBackColor();
        _styles[i].style = 0; // no underline, italic, bold     
    }
    
    _styles[style_name_tag - 1].normalForeColor = fxcolorfromname("Aquamarine4");
    _styles[style_name_tag - 1].style           = STYLE_BOLD;

    _styles[style_name - 1].normalForeColor     = fxcolorfromname("Brick");

    _styles[style_name_notauth - 1].normalForeColor = fxcolorfromname("Red");
    _styles[style_name_notauth - 1].style           = STYLE_BOLD;
    _styles[style_text_notauth - 1].normalForeColor = fxcolorfromname("Red");

    _styles[style_status - 1].normalForeColor = fxcolorfromname("Red");

    _styles[style_url - 1].normalForeColor = fxcolorfromname("Blue");
    _styles[style_url - 1].style           = STYLE_UNDERLINE;

    setStyled(true);
    setHiliteStyles(_styles);
    // public_message("Myself", "Tadaa!");
}

chat::~chat() {
    delete [] _styles;
}

void
chat::create() {
    FXText::create();
    // show(PLACEMENT_SCREEN);
}

long
chat::on_input(FXObject *from, FXSelector sel, void *) {
    // TODO later... maybe.
    return 0;
}

long
chat::on_motion(FXObject *from, FXSelector sel, void *ptr) {    
    FXEvent *event = (FXEvent*)ptr;
    FXint pos = getPosAt(event->win_x, event->win_y);
    
    // ACE_DEBUG((LM_DEBUG, "chat::on_motion %d/%d pos %d/%d\n",
    //          event->win_x, event->win_y, pos, getLength())); 

    // Default is arrow cursor.
    FXDefaultCursor set_cursor = DEF_ARROW_CURSOR;  
    if (pos < getLength()) {   
        FXint style = getStyle(pos);
        if (style == style_url) {
            set_cursor = DEF_HAND_CURSOR;
        }
    }

    if (_current_cursor != set_cursor) {
        setDefaultCursor(getApp()->getDefaultCursor(set_cursor));
        _current_cursor = set_cursor;
    }
    
    FXText::handle(from, sel, ptr); 
    return 0;
}

long
chat::on_left_button(FXObject *from, FXSelector sel, void *ptr) {   
    FXEvent *event = (FXEvent*)ptr;
    FXint pos = getPosAt(event->win_x, event->win_y);
    
    if (pos >= getLength()) return 0;
    
    FXint style = getStyle(pos);
    
    if (style == style_url) {
        // Read in the whole URL. First find the start
        for (; pos >= 0 && getStyle(pos) == style_url; pos--);
        
        pos++;
        // Then construct the URL
        std::string link;
        for (; pos < getLength() && getStyle(pos) == style_url; pos++) {
            link += getChar(pos);
        }

        ACE_DEBUG((LM_DEBUG, "chat::launching link %s\n", link.c_str()));
        
        getApp()->beginWaitCursor();
        if (!launcher_file()->start(link)) {
            getApp()->addTimeout(this,ID_TIMER,2000); // 2 seconds of away cursor
        } else {
            getApp()->endWaitCursor();
            getApp()->beep();
        }
    } else {
        FXText::handle(from, sel, ptr);
    }
    return 0;
}

long 
chat::on_timer(FXObject*,FXSelector,void* ptr) {
   getApp()->endWaitCursor();
   return 1;
}

long 
chat::on_configure(FXObject*,FXSelector,void* ptr) {
   FXWindow *win = this;
   ACE_DEBUG((LM_DEBUG, "chat::on_configure %d/%d/%d/%d\n", 
              win->getX(),win->getY(),win->getWidth(), win->getHeight()));
   _cond_scroll_prepare();
   _cond_scroll();
   // getApp()->endWaitCursor();
   return 1;
}

void
chat::handle_message(::message *msg) {
    switch (msg->id()) {
    case ::message::send:
    case ::message::send_private:
    {
        message_channel *m = dynamic_ptr_cast<message_channel>(msg);
        ACE_DEBUG((LM_DEBUG, "chat::handle message _channel/channel %s/%s\n",
                  _channel.c_str(), m->channel().c_str()));
                  
        if (_channel == m->channel()) {                
            // Public message
            public_message(m->sender_id(), m->str(), m->group_base());
        }
    }
        break;
    case ::message::private_refused:
    {
        message_channel *m = dynamic_ptr_cast<message_channel>(msg);
        ACE_DEBUG((LM_DEBUG, "chat::handle message _channel/channel %s/%s\n",
                  _channel.c_str(), m->channel().c_str()));
                  
        if (_channel == m->channel()) {
            // Status message of refused
            status_message(
                langstr("chat/private_msg_refused", m->str().c_str())
            );
        }
    }
        break;
    case ::message::block_users:
    {
        message_block_users *m = dynamic_ptr_cast<message_block_users>(msg);
        if (m->global_ignore() == false) {
            ACE_DEBUG((LM_DEBUG, "chat::handle message block_users\n"));
            status_message(
                langstr("chat/user_ignores", m->user().display_id().c_str())
            );
        }
    }
        break;
    case ::message::global_ignore_list:
        _handle_global_ignore(msg);
        break;
    case ::message::room:
    {
        message_room *m = dynamic_ptr_cast<message_room>(msg);
        const chat_gaming::room &r = m->room();

    if (app_opts.debug()) {
        std::string msgstr = 
            "Room update id " + (const std::string)r.id() + 
            " topic "         + r.topic() +
            " owner id "      + (const std::string)(r.owner_id());
        status_message(msgstr);
    }
    
    }
        break;
    case ::message::user:
    {
        message_user *m = dynamic_ptr_cast<message_user>(msg);
        const chat_gaming::user &u = m->user();

    if (app_opts.debug()) {
        std::string msgstr = 
            "User update id " + (std::string)u.id() + 
            " in room id "    + (std::string)u.room_id();
        status_message(msgstr);
    }
    
        
    }
        break;
        
    }
}

void 
chat::public_message(
  const chat_gaming::user::id_type &from_id, 
  const std::string &msg,
  int grp) 
{
    model::house::house_type::user_iterator ui 
      = house_model()->user_find(from_id, grp);
    if (ui == house_model()->user_end()) {
        ACE_DEBUG((LM_ERROR, "chat::public_message no user found for "
        "user id %s\n", from_id.c_str()));
        return;
    }
    const chat_gaming::user &from = *ui;
    
    FXColor s_name = (from.authenticated() ? style_name : style_name_notauth);
    FXColor s_text = (from.authenticated() ? style_text : style_text_notauth);

    _cond_scroll_prepare();
    {   
        /** Message Header **/
        appendStyledText("<", 1, style_name_tag);
        appendStyledText(from.display_id().c_str(), from.display_id().size(), s_name);
        appendStyledText(">", 1, style_name_tag);
        appendStyledText(" ", 1);
    
        /** Message Contents **/
        _styled_content(msg.c_str(), msg.size(), s_text);
        appendStyledText("\n", 1);  
    }
    _cond_scroll();
    
    if (_flash_window && app_opts.flash_nick()) {
        ACE_DEBUG((LM_DEBUG, "chat::public_message: trying to find %s from %s\n",
                  self_model()->user().display_id().c_str(), msg.c_str()));
        if (regexp::matchiv(self_model()->user().display_id(), msg)) {
            ACE_DEBUG((LM_DEBUG, "chat::public_message: found\n"));
            os::flash_window(_flash_window); 
        }
    }
}

void 
chat::status_message(const std::string &msg) {
    _cond_scroll_prepare();
    {
        /** Message Header **/
        appendStyledText("*", 1, style_status);
        appendStyledText(" ", 1);
    
        /** Message Contents **/
        appendStyledText(msg.c_str(), msg.size(), style_status);
        appendStyledText("\n", 1);
    }   
    _cond_scroll();
}

void 
chat::private_message(const FXString &fromUser, const FXString &msg) {
    // public_message(fromUser, msg);
}

void 
chat::_cond_scroll_prepare() {
    // Chack that if the last line before new text is entered is not visible,
    // then do not do the scrolling because it means user has scrolled the
    // buffer back and wants to view older text.
    
    FXint last_pos = getLength() - 1;
    // Use rowStart() to find the position of the start of the row
    // FXint last_row_start_pos = rowStart(last_pos);
    _allow_scroll = isPosVisible(last_pos);
}

void
chat::_cond_scroll() {  
    int cx, cy;
    getPosition(cx, cy);
    // ACE_DEBUG((LM_DEBUG, "chat::_cond_scroll: curr x/y: %d/%d\n", cx, cy));
    if (_allow_scroll && getContentHeight() > getViewportHeight()) {
        // ACE_DEBUG((LM_DEBUG, "chat::_cond_scroll: do scroll "
        // "content/viewport height: %d/%d\n",
        // getContentHeight(), getViewportHeight()));
        
        // Fox uses negative Y for down below???
        setPosition(0, getViewportHeight() - getContentHeight());
    }
}

void
chat::_styled_content(const char *msg, size_t len, int base_style) {
    ACE_DEBUG((LM_DEBUG, "chat::_styled_content\n"));
    const char *guard = msg + len;
    const char *base  = msg;
    const char *p     = msg;
    
    parser::url::dfa url_parser;
    
    while (p < guard) {
        // Eat separator characters away
        for (; p < guard; p++) {
            if (is_word_separator(*p)) continue;
            else break;
        }
        
        parser::url::dfa::status status = url_parser.parse(p, guard);
        if (status == parser::url::dfa::success) {
            if (base != p)
                appendStyledText(base, (int)(p - base), base_style);
            base = p;
            p    = url_parser.end();
            appendStyledText(base, (int)(p - base), style_url);
            base = p;
        } else {
            // Eat until separator character
            for (; p < guard; p++) {
                if (is_word_separator(*p)) break;
                else continue;
            }
            appendStyledText(base, (int)(p - base), base_style);
            base = p;
        }
    }   
}

void
chat::_handle_global_ignore(::message *msg) {
    ACE_DEBUG((LM_DEBUG, "chat::_handle_global_ignore"));
    message_global_ignore *m = dynamic_ptr_cast<message_global_ignore>(msg);
    message_global_ignore::ip_list_type::const_iterator i = m->ip_begin();
    
    bool first = true;
    for (; i != m->ip_end(); i++) {
        if (first) {
            status_message(langstr("chat/global_ignore"));
            first = false;
        }
        status_message(
            langstr("chat/user_global_ignore", 
                    i->userid.c_str(), i->reason.c_str())
        );        
    }
}

} // ns view
} // ns gui
