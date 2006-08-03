#ifndef _MULTI_FEED_COMMON_H_
#define _MULTI_FEED_COMMON_H_

#include <string>
#include <map>

#include "../chat_gaming/user.h"

namespace multi_feed {

typedef std::map<int, std::string>                feed_value_map_type;

#if 0
typedef std::map<int, chat_gaming::user::id_type> feed_user_id_map_type;
#endif

}

#endif /*_MULTI_FEED_COMMON_H_*/
