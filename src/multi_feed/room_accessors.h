#ifndef _MULTI_FEED_ROOM_ACCESSORS_H_
#define _MULTI_FEED_ROOM_ACCESSORS_H_

#include <string>
#include <sstream>
#include <functional>
#include <iterator>

#include <netcomgrp/common.h>

#include "../common.h"
#include "../chat_gaming/room.h"
#include "../chat_gaming/user.h"
#include "../model/house.h"
#include "../main.h"

#include "common.h"
#include "algorithm.h"

namespace multi_feed {

// General user value accessor object, not usually used directly
template <class ValueAccessor>
class room_value_by_id_accessor_fobj {
    ValueAccessor _acc;
public:
    inline room_value_by_id_accessor_fobj(
        ValueAccessor accessor
    ) : _acc(accessor) {}

    std::string operator()(const chat_gaming::room::id_type &rid, int grp) {
        model::house::house_type::room_iterator ri =
            house_model()->room_find(rid, grp);
        if (ri == house_model()->room_end()) return std::string();
        const model::house::room_type &r = *ri;
        const std::string &s = _acc(r, grp);
        ACE_DEBUG((LM_DEBUG, "multi_feed::room_value_accessor::value: %d value: '%s'\n",
                   grp, s.c_str()));
        return s;
    }
};

// General user value accessor object, not usually used directly
template <class AccRet>
class room_value_accessor_by_ptr_fobj {
    AccRet (chat_gaming::room::*_acc)() const;
public:
    inline room_value_accessor_by_ptr_fobj(
        AccRet (chat_gaming::room::*accessor)() const
    ) : _acc(accessor) {}

    inline
    AccRet operator()(const chat_gaming::room &r) {
        return (r.*_acc)();
    }
};

class room_owner_id_accessor_fobj {
public:
    inline
    const chat_gaming::user::id_type &operator()(const chat_gaming::room &r) {
        return r.owner_id();
    }
};

template <class AccRet, class RetTrans>
inline
room_value_by_id_accessor_fobj<
value_to_string_fobj<
chat_gaming::room,
room_value_accessor_by_ptr_fobj<AccRet>,
RetTrans
>
>
room_value_accessor(
    AccRet (chat_gaming::room::*accessor)() const,
    RetTrans trans
) {
    typedef
    value_to_string_fobj<
    chat_gaming::room,
    room_value_accessor_by_ptr_fobj<AccRet>,
    RetTrans
    > accessor_fobj;

    return
        room_value_by_id_accessor_fobj<accessor_fobj>(
            accessor_fobj(
                room_value_accessor_by_ptr_fobj<AccRet>(
                    accessor
                ),
                trans
            )
        );
}

template <class AccRet>
inline
room_value_by_id_accessor_fobj<
value_to_string_fobj<
chat_gaming::room,
room_value_accessor_by_ptr_fobj<AccRet>,
dummy_string_to_string
>
>
room_value_accessor(
    AccRet (chat_gaming::room::*accessor)() const
) {
    return room_value_accessor(accessor, dummy_string_to_string());
}

template <class RetTrans>
inline
room_value_by_id_accessor_fobj<
value_to_string_fobj<
chat_gaming::room,
room_owner_id_accessor_fobj,
RetTrans
>
>
room_owner_id_accessor(
    RetTrans trans
) {
    typedef
    value_to_string_fobj<
    chat_gaming::room,
    room_owner_id_accessor_fobj,
    RetTrans
    > accessor_fobj;

    return
        room_value_by_id_accessor_fobj<accessor_fobj>(
            accessor_fobj(
                room_owner_id_accessor_fobj(),
                trans
            )
        );
}

} // multi_feed

#endif /*_MULTI_FEED_ROOM_ACCESSORS_H_*/
