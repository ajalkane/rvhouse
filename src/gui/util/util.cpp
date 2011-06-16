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
