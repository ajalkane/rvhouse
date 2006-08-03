#ifndef _CHAT_GAMING_USER_KEY_H_
#define _CHAT_GAMING_USER_KEY_H_

#include <boost/serialization/access.hpp>

#include <string>

#include <netcomgrp/node.h>

namespace chat_gaming {
    class user_key {
        const netcomgrp::node *_node;
        std::string            _id_str;
        
        friend class boost::serialization::access;
        
        // When the class Archive corresponds to an output archive, the
        // & operator is defined similar to <<.  Likewise, when the class Archive
        // is a type of input archive the & operator is defined similar to >>.
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & _id_str;
            // Node pointer is not serialized on purpose
        }       
        
    public: 
        inline user_key();
        inline user_key(const netcomgrp::node *n);
        inline user_key(const std::string &str);
        inline user_key(const user_key &o);
        inline ~user_key();
        
        inline const std::string &id_str(const std::string &);
        inline const std::string &id_str() const;

        // std::string compatibility function(s)
        inline const char *c_str() const;
        inline operator const std::string&() const;
        inline bool empty() const {
            return _node == NULL && _id_str.empty();
        }
        
        inline const netcomgrp::node *node() const;
        inline const netcomgrp::node *node(const netcomgrp::node *);
        
        inline user_key &operator=(const user_key &o);
        inline bool operator==(const user_key &o) const;
        inline bool operator!=(const user_key &o) const { return !(*this==o); }
        inline bool operator<(const user_key &o) const;
    };

    user_key::user_key() : _node(NULL) {}
    user_key::user_key(const netcomgrp::node *n)
      : _node(n ? n->duplicate() : NULL) {
        ACE_DEBUG((LM_DEBUG, "user_key::constructor robot node %d/%d\n", n, _node));
    }

    user_key::user_key(const std::string &str)
      : _node(NULL), _id_str(str) {
    }

    user_key::user_key(const user_key &o) 
      : _node(o._node ? o._node->duplicate() : NULL),
        _id_str(o._id_str) 
    {
        ACE_DEBUG((LM_DEBUG, "user::copy constructor robot node %d\n", _node));
    }

    user_key::~user_key() {
        ACE_DEBUG((LM_DEBUG, "user_key::dtor node %d\n", _node));
        delete _node;
    }

    inline const std::string &
    user_key::id_str(const std::string &s) { return _id_str = s; }
    inline const std::string &
    user_key::id_str() const { return _id_str; }

    inline const char *
    user_key::c_str() const {
        if (!_id_str.empty()) return _id_str.c_str();
        if (_node) {
            return "<NODE PTR SET>";
        }
        return "<NO NODE PTR SET>";
    }
    inline user_key::operator const std::string&() const {
        return _id_str;
    }

    inline const netcomgrp::node *
    user_key::node() const { return _node; }
    
    inline const netcomgrp::node *
    user_key::node(const netcomgrp::node *n) {
        delete _node; // Delete old if it existed
        return _node = (n ? n->duplicate() : NULL);
    }

    inline bool 
    user_key::operator==(const user_key &o) const {
        // if (!_node || !o._node) return _id_str == o._id_str;
        if (!_node || !o._node) return false;
        return *_node == *o._node; 
    }
    inline bool 
    user_key::operator<(const user_key &o) const {
        if (!_node)   return false;
        if (!o._node) return true;
        // if (!_node || !o._node) return _id_str < o._id_str;
        return *_node <  *o._node; 
    }   

    inline user_key &
    user_key::operator=(const user_key &o) {
        ACE_DEBUG((LM_DEBUG, "user::operator= node (%d/%d)\n", _node, o._node));

        this->node(o.node());
        this->id_str(o.id_str());
                
        return *this;
    }   
    
} // ns chat_gaming

#endif //_CHAT_GAMING_USER_KEY_H_
