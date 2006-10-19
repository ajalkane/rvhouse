#ifndef _GUI_VIEW_CHAT_H_
#define _GUI_VIEW_CHAT_H_

#include <string>

#include <fx.h>
#include <FXText.h>

#include "../../chat_gaming/user.h"
#include "../../common.h"
#include "../../messaging/message.h"

namespace gui {
namespace view {

class chat : public FXText {
    FXDECLARE(chat)
    
    std::string _channel;
        
protected:
    chat() {}
    enum {
        style_name_tag = 1,
        style_name,
        style_name_notauth,
        style_text,
        style_text_notauth,
        style_status,
        style_url,
        style_last,
    };
    FXDefaultCursor _current_cursor;
    FXHiliteStyle  *_styles;
    FXWindow       *_flash_window;
    
    bool _allow_scroll;
    void _cond_scroll();
    void _cond_scroll_prepare();
    void _styled_content(const char *msg, size_t len, int base_style);
public:
    enum {
        ID_INPUT = FXText::ID_LAST,
        ID_TIMER,
        ID_LAST,
    };
    
    chat(FXComposite *c, FXObject *tgt=NULL, 
         FXSelector sel=0, 
         FXuint opts=TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y, 
         FXint x=0, FXint y=0, 
         FXint w=0, FXint h=0, 
         FXint pl=3, FXint pr=3, 
         FXint pt=2, FXint pb=2);
    virtual ~chat();
    virtual void create();
    
    inline void flash_window(FXWindow *w) { _flash_window = w; }
    
    inline const std::string &channel(const std::string &s) {
        
        return _channel = s;
    }
    inline const std::string &channel() const {
        return _channel;
    }
    void public_message (const chat_gaming::user::id_type &user_id, 
                         const std::string &msg, int grp); 
    void status_message (const std::string &msg); 
    void private_message(const FXString &fromUser, const FXString &msg);
    
    long on_input      (FXObject *from, FXSelector sel, void *);    
    long on_motion     (FXObject *from, FXSelector sel, void *);    
    long on_left_button(FXObject *from, FXSelector sel, void *);    
    long on_timer      (FXObject *from, FXSelector sel, void *);    
    void handle_message(::message *msg);
};

} // ns view
} // ns gui

#endif //_GUI_VIEW_CHAT_H_
