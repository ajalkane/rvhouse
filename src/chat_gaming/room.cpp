#include <sstream>
#include <stdlib.h>

#include "room.h"

namespace chat_gaming {

void
room::generate_id() {
    // Generate an id for the created room... just use the id of the
    // owner and timestamp to generate a reasonably unique ID
    std::ostringstream room_id;
    room_id << time(NULL) << ":" << owner_id().id_str();
    ACE_DEBUG((LM_DEBUG, "chat_gaming::room: generated id: %s\n",
               room_id.str().c_str()));

    super::id(room_id.str());
}

}
