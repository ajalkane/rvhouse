#include <sstream>

#include "../../regexp.h"

#include <QtGui>

#include "../../app_options.h"
#include "../../main.h"
#include "../../util.h"
#include "../../messaging/message_channel.h"
#include "../../messaging/message_room.h"
#include "../../messaging/message_user.h"
#include "../../messaging/message_block_users.h"
#include "../../messaging/message_global_ignore.h"
#include "../../model/house.h"
#include "../../model/self.h"
#include "../../parser/url/dfa.h"
#include "../../executable/launcher.h"
#include "../../os_util.h"

#include "chat.h"

namespace gui {
namespace view {

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

chat::chat(QWidget *parent, const std::string &channel)
: QTextBrowser(parent),
  _channel(channel),
  _flash_window(NULL),
  _allow_scroll(true)
{
    this->setReadOnly(true);
    this->setAcceptRichText(false);
    this->setTabChangesFocus(true);
    this->setOpenExternalLinks(true);

    _styles = new QTextCharFormat[style_last];
    _styles[style_name_tag].setForeground(QColor(69, 139, 116, 255));
    _styles[style_name_tag].setFontWeight(QFont::Bold);

    _styles[style_name].setForeground(QColor(156, 102, 31, 255));

    _styles[style_name_notauth].setForeground(QColor(255, 0, 0, 255));
    _styles[style_name_notauth].setFontWeight(QFont::Bold);

    _styles[style_status].setForeground(QColor(255, 0, 0, 255));
    _styles[style_url].setAnchor(true);
    _styles[style_url].setUnderlineStyle(QTextCharFormat::SingleUnderline);
    _styles[style_url].setForeground(QColor("blue"));

    // Since the message writing box has always focus, copying text from chat
    // is enabled automatically whenever text is selected. Perhaps not optimal,
    // but easy to implement and quite convenient once gotten used to.
    connect(this, SIGNAL(selectionChanged()), this, SLOT(copy()));
}

chat::~chat() {
    delete [] _styles;
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
    case ::message::send_notification:
    {
        message_channel *m = dynamic_ptr_cast<message_channel>(msg);
        ACE_DEBUG((LM_DEBUG, "chat::handle message _channel/channel %s/%s\n",
                  _channel.c_str(), m->channel().c_str()));

        if (_channel == m->channel()) {
            notification_message(
                m->sender_id(), m->str(), m->group_base()
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

    _style s_name = (from.authenticated() ? style_name : style_name_notauth);
    _style s_text = (from.authenticated() ? style_text : style_text_notauth);
    _cond_scroll_prepare();
    {
        _insert_text("<", style_name_tag);
        _insert_text(from.display_id().c_str(), s_name);
        _insert_text(">", style_name_tag);
        _insert_text(" ", s_text);
        _styled_content(msg, s_text);
        _insert_text("\n", s_text);
    }
    _cond_scroll();

    ACE_DEBUG((LM_DEBUG, "chat::public_message: flash_window %d, flash_nick %d\n",
               _flash_window, app_opts.flash_nick()));

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
chat::_insert_text(const char *text, const QTextCharFormat &format) {
    ACE_DEBUG((LM_ERROR, "chat::_insert_text '%s'\n", text));
    QTextCursor c = this->textCursor();
    c.insertText(text, format);
}

void
chat::_insert_text(const char *text, enum _style style) {
    QTextCharFormat format = _styles[style];
    _insert_text(text, format);
}

void
chat::notification_message(
  const chat_gaming::user::id_type &from_id,
  const std::string &msg,
  int grp)
{
    model::house::house_type::user_iterator ui
      = house_model()->user_find(from_id, grp);
    if (ui == house_model()->user_end()) {
        ACE_DEBUG((LM_ERROR, "chat::notification_message no user found for "
        "user id %s\n", from_id.c_str()));
        return;
    }
    const chat_gaming::user &from = *ui;

    _cond_scroll_prepare();
    {
        /** Message Header **/
        // At least for now display notification messages just like status messages
        _insert_text("*", style_status);
        _insert_text(" ", style_status);

        _insert_text("<", style_status);
        _insert_text(from.display_id().c_str(), style_status);
        _insert_text(">", style_status);
        _insert_text(" ", style_status);

        /** Message Contents **/
        _styled_content(msg, style_status);
        _insert_text("\n", style_status);
    }
    _cond_scroll();
}

void
chat::status_message(const std::string &msg) {
    _cond_scroll_prepare();
    {
        /** Message Header **/
        _insert_text("*", style_status);
        _insert_text(" ", style_status);

        /** Message Contents **/
        _insert_text(msg.c_str(), style_status);
        _insert_text("\n", style_status);
    }
    _cond_scroll();
}

void
chat::_cond_scroll_prepare() {
    // Chack that if the last line before new text is entered is not visible,
    // then do not do the scrolling because it means user has scrolled the
    // buffer back and wants to view older text.
    QScrollBar *vbar = verticalScrollBar();
    int maximum = vbar->maximum();
    int value = vbar->value();
    int single_step = vbar->singleStep();
    _saved_pre_insert_vertical_value = value;

    _allow_scroll = (maximum - value < single_step);

    ACE_DEBUG((LM_DEBUG, "chat::_cond_scroll_prepare: max %d, val %d, singleStep %d, allow_scroll: %d\n", maximum, value, single_step, _allow_scroll));

    // Ensure text is inserted at the end of the buffer
    QTextCursor c = this->textCursor();
    c.movePosition(QTextCursor::End);
    setTextCursor(c);
}

void
chat::_cond_scroll() {
    if (_allow_scroll) {
        ACE_DEBUG((LM_DEBUG, "chat::_cond_scroll: scrolling to visible\n"));
        ensureCursorVisible();
    } else {
        ACE_DEBUG((LM_DEBUG, "chat::_cond_scroll: not scrolling to visible\n"));
        verticalScrollBar()->setValue(_saved_pre_insert_vertical_value);
    }
}

void
chat::_styled_content(const std::string &msg, enum _style text_style) {
    ACE_DEBUG((LM_DEBUG, "chat::_styled_content\n"));
    const char *base  = msg.c_str();
    const char *guard = base + msg.size();
    const char *p     = base;

    parser::url::dfa url_parser;

    while (p < guard) {
        // Eat separator characters away
        for (; p < guard; p++) {
            if (is_word_separator(*p)) continue;
            else break;
        }

        parser::url::dfa::status status = url_parser.parse(p, guard);
        if (status == parser::url::dfa::success) {
            if (base != p) {
                std::string non_url(base, (size_t)(p - base));
                _insert_text(non_url.c_str(), text_style);
            }
            base = p;
            p    = url_parser.end();
            _insert_url(base, (int)(p - base));
            base = p;
        } else {
            // Eat until separator character
            for (; p < guard; p++) {
                if (is_word_separator(*p)) break;
                else continue;
            }
            std::string non_url(base, (size_t)(p - base));
            _insert_text(non_url.c_str(), text_style);
            base = p;
        }
    }
}

void
chat::_insert_url(const char *url, size_t len) {
    std::string stdurl(url, len);
    ACE_DEBUG((LM_DEBUG, "chat::_insert_url: raw url '%s'\n", stdurl.c_str()));
    QTextCharFormat url_style(_styles[style_url]);
    url_style.setAnchor(true);
    url_style.setAnchorHref(stdurl.c_str());
    _insert_text(stdurl.c_str(), url_style);
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
