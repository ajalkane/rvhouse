#ifndef _EXECUTABLE_IMPL_LAUNCHER_DX7_H_
#define _EXECUTABLE_IMPL_LAUNCHER_DX7_H_

#include <dplay.h>
#include <dplobby.h>

#include <map>
#include <string>
#include <list>

#include "../launcher.h"

namespace executable {

class launcher_dx7 : public launcher {
    LPDIRECTPLAYLOBBY3A  _lobby;
    
    struct _service_provider {
        std::string name;
        GUID        guid;
    };
    
    // Maps the application name to it's GUID
    typedef std::map<std::string, GUID> _app_guid_map_type;
    _app_guid_map_type _app_guid_map;
    // List of service providers offering internet facility
    // typedef std::list<_service_provider> _sp_list_type;
    // _sp_list_type _sp_inet_list;
    
    static const GUID  _app_session_guid;
    static const char *_app_session_name;

    static BOOL FAR PASCAL 
      _collect_registered_apps_cb(const DPLAPPINFO* pAppInfo, VOID* pContext,
                                  DWORD dwFlags);
    static BOOL FAR PASCAL 
      _collect_service_providers_cb( 
        const GUID* pSPGUID, VOID* pConnection,
        DWORD dwConnectionSize, const DPNAME* pName,
        DWORD dwFlags, VOID* pContext);
        
    void _init();
    int  _run_app(const std::string &app, bool is_host, 
                  const std::string &host_addr = std::string());
    void _create_dx_addr(const std::string &host_addr, 
                         VOID **dx_ret_addr, DWORD *dx_ret_size);
public:
    launcher_dx7();
    launcher_dx7(const std::string &default_app);
    
    virtual ~launcher_dx7();
    
    virtual int start(const std::string &app) { return err_not_supported; }
    virtual int start_host(const std::string &app);
    virtual int start_client(const std::string &app, const std::string &host_id);   
};

} // ns executable

#endif //_EXECUTABLE_IMPL_LAUNCHER_DX7_H_
