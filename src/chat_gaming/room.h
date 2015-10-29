#ifndef _CHAT_GAMING_ROOM_H_
#define _CHAT_GAMING_ROOM_H_

#include "../common.h"

#include <boost/serialization/access.hpp>

#include <string>
#include <chat/room.h>

#include "user_key.h"

namespace chat_gaming {
    class room : public chat::room<std::string, user_key> {
        typedef chat::room<std::string, user_key> super;

        unsigned    _sequence;      
        std::string _topic;
        std::string _password;
        size_t      _laps;
        size_t      _max_players;
        bool        _pickups;
        bool        _has_password;
        bool        _version_rvgl;
        
        friend class boost::serialization::access;
        
        // When the class Archive corresponds to an output archive, the
        // & operator is defined similar to <<.  Likewise, when the class Archive
        // is a type of input archive the & operator is defined similar to >>.
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & _sequence;
            ar & _id;
            ar & _owner_id;
            ar & _topic;
            ar & _laps;
            ar & _max_players;
            ar & _pickups;
            ar & _has_password;
            ar & _version_rvgl;
        }       
        
    public: 
        inline room() : _sequence(0), _max_players(0), _has_password(false) {};
        inline room(const id_type &i) : super(i) {}

        inline unsigned sequence() const     { return _sequence; }
        inline unsigned sequence(unsigned s) { return _sequence = s; }
        
        inline const std::string &topic() const { return _topic; }
        inline const std::string &topic(const std::string &t) {
            return _topic = t;
        }

        inline const std::string &password() const { return _password; }
        inline const std::string &password(const std::string &p) {
            _has_password = (p.empty() ? false : true);
            return _password = p;
        }

        inline bool has_password() const { return _has_password; }
        // inline bool has_password(int p)  { return _has_password = p; }
        
        inline size_t laps() const { return _laps; }
        inline size_t laps(int m)  { return _laps = m; }

        inline size_t max_players() const { return _max_players; }
        inline size_t max_players(int m)  { return _max_players = m; }

        inline bool pickups() const { return _pickups; }
        inline bool pickups(bool p)  { return _pickups = p; }

        inline bool version_rvgl() const { return _version_rvgl; }
        inline bool version_rvgl(bool v) { return _version_rvgl = v; }
        
        // Generates and sets user::id from owner and timestamp
        void generate_id();         
    };

} // ns chat

#endif //_CHAT_GAMING_ROOM_H_
