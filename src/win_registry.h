#ifndef _WIN_REGISTRY_H_
#define _WIN_REGISTRY_H_

// TODO rewrite to have common interface, a windows specific version,
// and linux specific version...

#include <string>
#include <sstream>
#include <locale>
#include <algorithm>

#ifdef WIN32
#include <windows.h>
#endif

#include "common.h"
#include "util.h"

// Class representing a registry subdirectory in Windows
class win_registry {
    bool _open;
#ifdef WIN32
    HKEY _parent;

    // Disables space as a white space separator. Needed so that
    // can extract nicely also strings that have white spaces in them.
    // Modified from the example found from here:
    // http://www.talkaboutprogramming.com/group/comp.lang.c++/messages/740314.html
    struct no_space_as_ws_ctype : std::ctype<char> {
        no_space_as_ws_ctype() : std::ctype<char>(get_table()) {}
        static std::ctype_base::mask const *get_table() {
            static std::ctype_base::mask *rc = 0;

            if (rc == 0) {
                rc = new std::ctype_base::mask[std::ctype<char>::table_size];
                std::fill_n(rc, std::ctype<char>::table_size,
                            std::ctype_base::mask());
                // Use only linefeed as a separator
                rc['\n'] = std::ctype_base::space;
            }
            return rc;
        }
    };  
#endif

public:
    enum shortcut_id {
        id_software = 1,
        id_uninstall,
        id_dplay
    };

    win_registry(const std::string &full_path);
    win_registry(shortcut_id id, 
                 const std::string &vendor,
                 const std::string &app,
                 const std::string &version = std::string());
    virtual ~win_registry();

    inline bool is_open() const { return _open; }

    template <class T>
    T get(const std::string &id, T def_val) {
#ifdef WIN32
        if (!_open) return def_val;
        
        char    buf[1024];
        DWORD   buf_len = array_sizeof(buf);
        LONG    res;
        DWORD   type;
        
        res = RegQueryValueEx(
            _parent, id.c_str(), NULL, &type,
            (LPBYTE) buf, &buf_len
        );

        if((res != ERROR_SUCCESS) || (buf_len > array_sizeof(buf))) {
            ACE_DEBUG((LM_DEBUG, "win_registry::get %s failed, "
                       "required size %d\n",
                       id.c_str(), buf_len));
            return def_val;
        }
        
        ACE_DEBUG((LM_DEBUG, "win_registry::get received value of size %d bytes\n",
                  buf_len));
        
        // Using a generic stringstream so that conversions between the
        // types is straightforward. Its a little inefficient, but its not
        // an issue in the contexts this class is used.
        std::stringstream stream;
        
        switch (type) {
        case REG_DWORD:
            stream << *((DWORD *)buf);
            break;
        case REG_SZ:
            stream << buf;
            break;
        default:
            ACE_DEBUG((LM_DEBUG, "win_registry::get path %s received "
                      "unrecognized type %d, returning default value\n",
                      id.c_str(), type));
            return def_val; 
        }

        ACE_DEBUG((LM_DEBUG, "win_registry::get key/value: %s/%s\n",
                  id.c_str(), stream.str().c_str()));
                  
        T ret_val;
        stream.imbue(std::locale(std::locale(), new no_space_as_ws_ctype));
        stream >> ret_val;
        return ret_val;
#else
        return def_val;
#endif
    }

    template <class T>
    void set(const std::string &id, const T &val) {
        if (!_open) return;

#ifdef WIN32        
        LONG    res;
        DWORD   type;
        
        // Query only the type of the value for conversion purposes
        res = RegQueryValueEx(
            _parent, id.c_str(), NULL, &type,
            NULL, NULL
        );

        if(res != ERROR_SUCCESS) {
            ACE_DEBUG((LM_DEBUG, "win_registry::set %s failed querying type\n",
                       id.c_str()));
            return;
        }
                
        // Using a generic stringstream so that conversions between the
        // types is straightforward. Its a little inefficient, but performance
        // is not an issue in the contexts this class is used.
        std::ostringstream reg_stream;
        
        switch (type) {
        case REG_DWORD:
        {
            ACE_DEBUG((LM_DEBUG, "win_registry::set DWORD target\n"));
            // Convert the value to binary DWORD
            DWORD v;
            std::stringstream stream(std::ios::in  |
                                     std::ios::out |
                                     std::ios::binary);
            stream << val;
            stream >> v;
            reg_stream.write(reinterpret_cast<const char *>(&v), sizeof(v));
            ACE_DEBUG((LM_DEBUG, "win_registry::set key/value: %s/%d\n",
                      id.c_str(), v));
        }
            break;
        case REG_SZ:
            ACE_DEBUG((LM_DEBUG, "win_registry::set string target\n"));
            // NULL termination must be calculated to the size, so add
            // it to the stream. This ensures the written value is NULL
            // terminated and correct size value is gotten for the stream.
            reg_stream << val << '\0';
            ACE_DEBUG((LM_DEBUG, "win_registry::set key/value: %s/%s\n",
                      id.c_str(), reg_stream.str().c_str()));
            break;
        default:
            ACE_DEBUG((LM_DEBUG, "win_registry::set path %s received "
                      "unrecognized type %d, returning default value\n",
                      id.c_str(), type));
            return; 
        }
        
        ACE_DEBUG((LM_DEBUG, "win_registry::set setting value of size %d\n",
                   reg_stream.str().size()));
                   
        res = RegSetValueEx(
            _parent, id.c_str(), 0, type,
            reinterpret_cast<const BYTE *>(reg_stream.str().data()), 
            reg_stream.str().size()
        );

        if(res != ERROR_SUCCESS) {
            ACE_DEBUG((LM_DEBUG, "win_registry::set %s failed\n",
                       id.c_str()));
            return;
        }       
#endif
    }   
};

#endif /* _WIN_REGISTRY_H_ */
