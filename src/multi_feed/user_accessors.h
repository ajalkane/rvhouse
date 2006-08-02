#ifndef _MULTI_FEED_USER_ACCESSORS_H_
#define _MULTI_FEED_USER_ACCESSORS_H_

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
class user_value_by_id_accessor_fobj {
    ValueAccessor _acc;
public:
    inline user_value_by_id_accessor_fobj(
        ValueAccessor accessor
    ) : _acc(accessor) {}

    std::string operator()(const chat_gaming::user::id_type &uid, int grp) {
        model::house::house_type::user_iterator ui =
            house_model()->user_find(uid, grp);
        if (ui == house_model()->user_end()) return std::string();
        const model::house::user_type &u = *ui;
        const std::string &s = _acc(u, grp);
        ACE_DEBUG((LM_DEBUG, "multi_feed::user_value_accessor::value: %d value: '%s'\n",
                   grp, s.c_str()));
        return s;
    }
};

// General user value accessor object, not usually used directly
template <class AccRet>
class user_value_accessor_by_ptr_fobj {
    AccRet (chat_gaming::user::*_acc)() const;
public:
    inline user_value_accessor_by_ptr_fobj(
        AccRet (chat_gaming::user::*accessor)() const
    ) : _acc(accessor) {}

    inline
    AccRet operator()(const chat_gaming::user &u) {
        return (u.*_acc)();
    }
};

class user_room_id_accessor_fobj {
public:
    inline
    std::string operator()(const chat_gaming::user &u) {
        return u.room_id();
    }
};

class user_id_accessor_fobj {
public:
    inline
    const chat_gaming::user::id_type &operator()(const chat_gaming::user &u) {
        return u.id();
    }
};

template <class RetTrans>
inline
user_value_by_id_accessor_fobj<
value_to_string_fobj<
chat_gaming::user,
self_accessor<chat_gaming::user>,
RetTrans
>
>
user_accessor(
    RetTrans trans
) {
    typedef
    value_to_string_fobj<
    chat_gaming::user,
    self_accessor<chat_gaming::user>,
    RetTrans
    > accessor_fobj;

    return
        user_value_by_id_accessor_fobj<accessor_fobj>(
            accessor_fobj(
                self_accessor<chat_gaming::user>(),
                trans
            )
        );
}

#if 0
template <class AccRet>
inline
user_value_by_id_accessor_fobj<
value_to_string_fobj<
chat_gaming::user,
user_value_accessor_by_ptr_fobj<AccRet>,
dummy_string_to_string
>
>
user_accessor(
    AccRet (chat_gaming::user::*accessor)() const
) {
    return user_accessor(accessor, dummy_string_to_string());
}
#endif

template <class AccRet, class RetTrans>
inline
user_value_by_id_accessor_fobj<
value_to_string_fobj<
chat_gaming::user,
user_value_accessor_by_ptr_fobj<AccRet>,
RetTrans
>
>
user_value_accessor(
    AccRet (chat_gaming::user::*accessor)() const,
    RetTrans trans
) {
    typedef
    value_to_string_fobj<
    chat_gaming::user,
    user_value_accessor_by_ptr_fobj<AccRet>,
    RetTrans
    > accessor_fobj;

    return
        user_value_by_id_accessor_fobj<accessor_fobj>(
            accessor_fobj(
                user_value_accessor_by_ptr_fobj<AccRet>(
                    accessor
                ),
                trans
            )
        );
}

template <class AccRet>
inline
user_value_by_id_accessor_fobj<
value_to_string_fobj<
chat_gaming::user,
user_value_accessor_by_ptr_fobj<AccRet>,
dummy_string_to_string
>
>
user_value_accessor(
    AccRet (chat_gaming::user::*accessor)() const
) {
    return user_value_accessor(accessor, dummy_string_to_string());
}

template <class RetTrans>
inline
user_value_by_id_accessor_fobj<
value_to_string_fobj<
chat_gaming::user,
user_room_id_accessor_fobj,
RetTrans
>
>
user_room_id_accessor(
    RetTrans trans
) {
    typedef
    value_to_string_fobj<
    chat_gaming::user,
    user_room_id_accessor_fobj,
    RetTrans
    > accessor_fobj;

    return
        user_value_by_id_accessor_fobj<accessor_fobj>(
            accessor_fobj(
                user_room_id_accessor_fobj(),
                trans
            )
        );
}

template <class RetTrans>
inline
user_value_by_id_accessor_fobj<
value_to_string_fobj<
chat_gaming::user,
user_id_accessor_fobj,
RetTrans
>
>
user_id_accessor(
    RetTrans trans
) {
    typedef
    value_to_string_fobj<
    chat_gaming::user,
    user_id_accessor_fobj,
    RetTrans
    > accessor_fobj;

    return
        user_value_by_id_accessor_fobj<accessor_fobj>(
            accessor_fobj(
                user_id_accessor_fobj(),
                trans
            )
        );
}

} // multi_feed

#endif /*_MULTI_FEED_USER_ACCESSORS_H_*/
