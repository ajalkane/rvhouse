#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#include "util.h"
#include "../../model/self.h"
#include "../../main.h"
#include "../../config_file.h"

namespace gui {
namespace util {

void store_size  (FXWindow *win, const char *section) {
    FXint x = win->getX(),
          y = win->getY(),
          w = win->getWidth(),
          h = win->getHeight();

    ACE_DEBUG((LM_DEBUG, "util::store_size: writing %s: %d/%d/%d/%d\n",
              section, x,y,w,h));

    if (w > 0 && h > 0) {          
        pref()->set(section, "x", x);
        pref()->set(section, "y", y);
        pref()->set(section, "w", w);
        pref()->set(section, "h", h);
    } else {
        ACE_DEBUG((LM_DEBUG, 
            "util::store_size: write skipped as w/h not valid\n"));
    }        
}

void restore_size(FXWindow *win, const char *section) {
    ACE_DEBUG((LM_DEBUG, "util::restore_size: %s\n", section));

    FXint x = pref()->get<FXint>(section, "x", 0),
          y = pref()->get<FXint>(section, "y", 0),
          w = pref()->get<FXint>(section, "w", -1),
          h = pref()->get<FXint>(section, "h", -1);
    
    if (w < 0 || h < 0) {
        ACE_DEBUG((LM_DEBUG, "util::restore_size: not set\n",
                  section));
        // In that case center the window
        win->setX(win->getRoot()->getWidth()/2  - win->getWidth()/2);
        win->setY(win->getRoot()->getHeight()/2 - win->getHeight()/2);
        return;
    }
    
    ACE_DEBUG((LM_DEBUG, "util::restore_size: x/y/w/h: %d/%d/%d/%d\n", 
              x,y,w,h));

    // Force the window to be on screen, just in case
    x = std::min(x, win->getRoot()->getWidth() - 50);
    y = std::min(y, win->getRoot()->getHeight() - 50);
    x = std::max(x, -w + 50);
    y = std::max(y, 50);
                  
    w = std::max(w, 30);
    h = std::max(h, 30);
    
    win->setX(x);
    win->setY(y);
    win->setWidth(w);
    win->setHeight(h);
}

std::string 
private_message_channel_with(const std::string &user_id) {
    // Build channel = combination of the recipient and our mighty self
    // By combining the user_ids in alphabetic order, can make sure
    // the channel id is the same on each side of the conversation. 
    std::vector<std::string> channel_keys(2);
    channel_keys[0] = user_id;
    channel_keys[1] = self_model()->user().id().id_str();
    std::sort(channel_keys.begin(), channel_keys.end());
    return channel_keys[0] + ":" + channel_keys[1];
    
}
    
} // ns util
} // ns gui
