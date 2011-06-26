#ifndef _CHAT_HOUSE_H_
#define _CHAT_HOUSE_H_

#include <set>
#include <list>
#include <map>
#include <utility>

namespace chat {
    template <typename UserType, 
              typename RoomType>
    class house {
    public:
        typedef UserType user_type;
        typedef RoomType room_type;
        
        typedef std::list<UserType> user_container_type;
        typedef std::list<RoomType> room_container_type;

        typedef typename user_container_type::iterator       user_iterator;
        typedef typename user_container_type::const_iterator const_user_iterator;
        typedef typename room_container_type::iterator       room_iterator;
        typedef typename room_container_type::const_iterator const_room_iterator;
        
        // TODO find/erase by id rather than by type
        inline user_iterator user_insert(const UserType &u);
        inline bool user_erase(const UserType &u);
        inline bool user_erase(const typename UserType::id_type &id);
        inline bool user_erase(user_iterator ui);
        inline user_iterator user_find(const UserType &u);
        inline user_iterator user_find(const typename UserType::id_type &u);
        inline user_iterator user_begin();
        inline user_iterator user_end();

        // TODO find/erase by id rather than by type
        inline room_iterator room_insert(const RoomType &u);
        inline bool room_erase(const RoomType &u);
        inline bool room_erase(const typename RoomType::id_type &id);
        inline bool room_erase(room_iterator ui);
        inline room_iterator room_find(const RoomType &u);
        inline room_iterator room_find(const typename RoomType::id_type &u);
        inline room_iterator room_begin();
        inline room_iterator room_end();

        template <typename IteratorType>
        int user_ids_in_room(const typename room_type::id_type &id, IteratorType out) {
            user_iterator i = user_begin();
            int count = 0;
            for (; i != user_end(); i++) {
                if (i->room_id() == id) {
                    *out++ = i->id();
                    count++;
                }
            }
            return count;
        }
        
    private:
        // typedef std::map<user_type, user_iterator> _user_key_type;
        typedef std::map<typename user_type::id_type, user_iterator> 
          _user_key_type;
        typedef std::map<typename room_type::id_type, room_iterator> 
          _room_key_type;
        // typedef std::map<typename user_container_type::iterator, 
        //                 typename room_container_type::iterator>
        //        _user_to_room_type;
                
        user_container_type _users;
        room_container_type _rooms;
        _user_key_type      _user_key;
        _room_key_type      _room_key;
        
    };

    template <typename UserType, typename RoomType> 
    inline typename house<UserType, RoomType>::user_iterator 
    house<UserType, RoomType>::user_insert(const UserType &u) {
        if (_user_key.count(u.id())) return user_end();
        user_iterator ui = _users.insert(_users.end(), u);
        // Id must be set separately as it is not usually copied
        ui->id(u.id());
        _user_key[u.id()] = ui; // .insert(ui);
        return ui;
    }

    template <typename UserType, typename RoomType> inline bool 
    house<UserType, RoomType>::user_erase(const UserType &u) {
        return user_erase(u.id());
    }

    template <typename UserType, typename RoomType> inline bool 
    house<UserType, RoomType>::user_erase(const typename UserType::id_type &id) {
        typename _user_key_type::iterator uki = _user_key.find(id);
        if (uki != _user_key.end()) {
            typename user_container_type::iterator ui = uki->second;
            _users.erase(ui);
            _user_key.erase(uki);
            return true;
        }
        return false;
    }

    template <typename UserType, typename RoomType> inline bool 
    house<UserType, RoomType>::user_erase(user_iterator uki) {
        return (uki != user_end() ? user_erase(*uki) : false);
    }

    template <typename UserType, typename RoomType> 
    inline typename house<UserType, RoomType>::user_iterator 
    house<UserType, RoomType>::user_find(const typename UserType::id_type &id) {
        typename _user_key_type::iterator uki = _user_key.find(id);
        return (uki != _user_key.end() ? uki->second : user_end());
    }
    template <typename UserType, typename RoomType> 
    inline typename house<UserType, RoomType>::user_iterator 
    house<UserType, RoomType>::user_find(const UserType &u) {
        return user_find(u.id());
    }

    template <typename UserType, typename RoomType> 
    inline typename house<UserType, RoomType>::user_iterator 
    house<UserType, RoomType>::user_begin() { return _users.begin(); }

    template <typename UserType, typename RoomType> 
    inline typename house<UserType, RoomType>::user_iterator 
    house<UserType, RoomType>::user_end()  { return _users.end(); }

    /**
     * ROOM
     */
    template <typename UserType, typename RoomType> 
    inline typename house<UserType, RoomType>::room_iterator 
    house<UserType, RoomType>::room_insert(const RoomType &r) {
        if (_room_key.count(r.id())) return room_end();
        room_iterator ri = _rooms.insert(_rooms.end(), r);
        // Id must be set separately as it is not usually copied
        ri->id(r.id());
        _room_key[r.id()] = ri;
        return ri;
    }

    template <typename UserType, typename RoomType> inline bool 
    house<UserType, RoomType>::room_erase(const RoomType &r) {
        return room_erase(r.id());
    }

    template <typename UserType, typename RoomType> inline bool 
    house<UserType, RoomType>::room_erase(const typename RoomType::id_type &id) {
        typename _room_key_type::iterator rki = _room_key.find(id);
        if (rki != _room_key.end()) {
            typename room_container_type::iterator ri = rki->second;
            _rooms.erase(ri);
            _room_key.erase(rki);
            return true;
        }
        return false;
    }

    template <typename UserType, typename RoomType> inline bool 
    house<UserType, RoomType>::room_erase(room_iterator rki) {
        return (rki != room_end() ? room_erase(rki->id()) : false);
    }

    template <typename UserType, typename RoomType> 
    inline typename house<UserType, RoomType>::room_iterator 
    house<UserType, RoomType>::room_find(const RoomType &r) {
        return room_find(r.id());
    }

    template <typename UserType, typename RoomType> 
    inline typename house<UserType, RoomType>::room_iterator 
    house<UserType, RoomType>::room_find(const typename RoomType::id_type &id) {
        typename _room_key_type::iterator rki = _room_key.find(id);
        return (rki != _room_key.end() ? rki->second : room_end());     
    } 

    template <typename UserType, typename RoomType> 
    inline typename house<UserType, RoomType>::room_iterator 
    house<UserType, RoomType>::room_begin()  { return _rooms.begin(); }

    template <typename UserType, typename RoomType> 
    inline typename house<UserType, RoomType>::room_iterator 
    house<UserType, RoomType>::room_end()  { return _rooms.end(); }
     
    
} // ns chat

#endif //_CHAT_HOUSE_H_
