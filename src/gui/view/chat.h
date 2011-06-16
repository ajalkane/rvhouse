#ifndef GUI_VIEW_CHAT_H_
#define GUI_VIEW_CHAT_H_

#include <string>

#include <QTextEdit>
#include <QTextBrowser>
#include <QWidget>
#include <QTextFormat>

#include "../../chat_gaming/user.h"
#include "../../common.h"
#include "../../messaging/message.h"

namespace gui {
namespace view {

class chat : public QTextBrowser {
    Q_OBJECT

    std::string _channel;

protected:
    chat() {}
    enum _style {
        style_name_tag = 0,
        style_name,
        style_name_notauth,
        style_text,
        style_text_notauth,
        style_status,
        style_url,
        style_last,
    };
    QTextCharFormat *_styles;
    QWidget         *_flash_window;
    int _saved_pre_insert_vertical_value;
    bool _allow_scroll;
    void _cond_scroll();
    void _cond_scroll_prepare();
    void _styled_content(const std::string &msg, _style style);
    void _insert_url(const char *url, size_t len);
    void _insert_text(const char *text, const QTextCharFormat &format);
    void _insert_text(const char *text, _style style);

    void _handle_global_ignore(::message *msg);
public:

    chat(QWidget *parent, const std::string &channel = std::string());
    virtual ~chat();

    inline void flash_window(QWidget *w) { _flash_window = w; }

    inline const std::string &channel() const {
        return _channel;
    }
    void public_message (const chat_gaming::user::id_type &user_id,
                         const std::string &msg, int grp);
    void notification_message (const chat_gaming::user::id_type &user_id,
                               const std::string &msg, int grp);
    void status_message (const std::string &msg);

    void handle_message(::message *msg);
};

} // ns view
} // ns gui

#endif //GUI_VIEW_CHAT_H_
