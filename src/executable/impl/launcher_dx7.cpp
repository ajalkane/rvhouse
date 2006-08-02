#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

#include "../../model/self.h"
#include "../../exception.h"
#include "launcher_dx7.h"

namespace executable {

const GUID launcher_dx7::_app_session_guid =
    { 0xcad2efec, 0x664d, 0x11da,
      { 0x95, 0x2a, 0x0, 0xe0, 0x81, 0x61, 0x16, 0x5f } };

// Hopefully a global session name is ok as only internet games are used.
const char *launcher_dx7::_app_session_name = ""; // house_launch";

// TODO it's not wise to do initialization here ... something like
// a singleton class that can thus be shared amongst many different classes
// that require DirectX functionality would be better. Implement
// if the need arises, for now this will do.
launcher_dx7::launcher_dx7() : _lobby(NULL) {
    _init();
}

launcher_dx7::launcher_dx7(const std::string &default_app)
        : launcher(default_app), _lobby(NULL) {
    _init();
}

void launcher_dx7::_init() {
    ACE_DEBUG((LM_DEBUG, "launcher_dx7: init\n"));
    HRESULT hr;
    LPDIRECTPLAY4A dp;

    // Init COM so we can use CoCreateInstance
    CoInitialize(NULL);

    // Create a temporary DirectPlay4 interface
    hr = CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_INPROC_SERVER,
                          IID_IDirectPlay4A, (VOID**)&dp);
    if (FAILED(hr))
        throw exception(0, "launcher_dx7: CoCreateInstance for "
                        "IID_IDirectPlay4A failed");

    hr = CoCreateInstance(CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER,
                          IID_IDirectPlayLobby3A, (VOID**)&_lobby );
    if (FAILED(hr)) {
        dp->Release();
        throw exception(0, "launcher_dx7: CoCreateInstance for "
                        "IID_IDirectPlayLobby3A failed");
    }

    _lobby->EnumLocalApplications(_collect_registered_apps_cb, this, 0);
    dp->EnumConnections(NULL, _collect_service_providers_cb, this, 0);

    dp->Release();

    // ACE_DEBUG((LM_DEBUG, "Force exit, testing phase\n"));
    // exit(1);
}

launcher_dx7::~launcher_dx7() {
    ACE_DEBUG((LM_DEBUG, "launcher_dx7: dtor\n"));

    if (_lobby) _lobby->Release();

    ACE_DEBUG((LM_DEBUG, "launcher_dx7: dtor exit\n"));
}

BOOL FAR PASCAL
launcher_dx7::_collect_registered_apps_cb(const DPLAPPINFO* app_info, VOID *ctx,
        DWORD flags)
{
    launcher_dx7 *self = reinterpret_cast<launcher_dx7 *>(ctx);

    ACE_DEBUG((LM_DEBUG, "launcher_dx7: found registered lobby app '%s'\n",
               app_info->lpszAppNameA));

    self->_app_guid_map[app_info->lpszAppNameA] = app_info->guidApplication;

    return TRUE;
}

BOOL FAR PASCAL
launcher_dx7::_collect_service_providers_cb(
    const GUID *sp_guid, VOID *connection,
    DWORD connection_size, const DPNAME *name_info,
    DWORD flags, VOID *ctx
) {
    // launcher_dx7 *self = reinterpret_cast<launcher_dx7 *>(ctx);
    ACE_DEBUG((LM_DEBUG, "launcher_dx7: found service provider '%s'\n",
               name_info->lpszShortName));

    // Nothing done for the time being, just debugging information
    return TRUE;
}


int
launcher_dx7::start_host(const std::string &app) {
    return _run_app(app, true);
}

int
launcher_dx7::start_client(const std::string &app, const std::string &host_id) {
    return _run_app(app, false, host_id);
}

int
launcher_dx7::_run_app(
    const std::string &app, bool is_host,
    const std::string &host_addr
) {
    HRESULT        hr;
    DPSESSIONDESC2 session_info;
    DPNAME		   player_name;
    DPLCONNECTION  connect_info;
    DWORD  dx_addr_size2 = 0;
    VOID  *dx_addr2      = NULL;
    char   player_name_str[64];
    char   session_name_str[64];

    ACE_DEBUG((LM_DEBUG, "launcher_dx7:_run_app\n"));

    snprintf(player_name_str, sizeof(player_name_str), "%s",
             self_model()->user().login_id().c_str());
    // Is this really necessary? Ie. can session_info struct modify
    // the string it gets? Play safe and do this here.
    snprintf(session_name_str, sizeof(session_name_str), "%s",
             _app_session_name);

    ACE_DEBUG((LM_DEBUG, "launcher_dx7:_run_app: player_name_str: %s\n", player_name_str));
    ACE_DEBUG((LM_DEBUG, "launcher_dx7:_run_app: session_name_str: %s\n", session_name_str));

    // Check if the app to run can be even found
    _app_guid_map_type::iterator app_i = _app_guid_map.find(app);
    if (app_i == _app_guid_map.end()) {
        return err_app_not_found;
    }

    // Create a dx lobby address from host_addr
    ACE_DEBUG((LM_DEBUG, "launcher_dx7::_create_dx_addr2 target pointers "
               "%d and %d\n", &dx_addr_size2, &dx_addr2));
    _create_dx_addr(host_addr, &dx_addr2, &dx_addr_size2);

    ZeroMemory(&session_info, sizeof(DPSESSIONDESC2));
    session_info.dwSize  = sizeof(DPSESSIONDESC2);
    session_info.guidInstance     = _app_session_guid;
    session_info.guidApplication  = app_i->second;
    session_info.lpszSessionNameA = session_name_str;

    // Fill out player name
    ZeroMemory(&player_name, sizeof(DPNAME));
    player_name.dwSize         = sizeof(DPNAME);
    player_name.lpszShortNameA = player_name_str;
    player_name.lpszLongNameA  = player_name_str;

    // Fill out connection description
    ZeroMemory(&connect_info, sizeof(DPLCONNECTION));
    connect_info.dwSize = sizeof(DPLCONNECTION);
    connect_info.lpSessionDesc = &session_info;
    connect_info.lpPlayerName  = &player_name;
    // Always use internet service provider, at least for now - don't really
    // see any advantage of supporting others from the lobby.
    connect_info.guidSP        = DPSPGUID_TCPIP;

    connect_info.lpAddress     = dx_addr2;
    connect_info.dwAddressSize = dx_addr_size2;

    connect_info.dwFlags = (is_host ? DPLCONNECTION_CREATESESSION
                            : DPLCONNECTION_JOINSESSION);

    ACE_DEBUG((LM_DEBUG, "launcher_dx7::_run_app: launching app\n"));
    DWORD app_id; // Only needed for RunApplication to fill
    hr = _lobby->RunApplication(0, &app_id, &connect_info, NULL);
    if (dx_addr2) GlobalFreePtr(dx_addr2);
    return (FAILED(hr) ? err_could_not_launch : 0);
}

void
launcher_dx7::_create_dx_addr(
    const std::string &host_addr,
    VOID **dx_ret_addr,
    DWORD *dx_ret_size)
{
    ACE_DEBUG((LM_DEBUG, "launcher_dx7::_create_dx_addr: creating dx addr from "
               "host address '%s'\n", host_addr.c_str()));
    DPCOMPOUNDADDRESSELEMENT addr_elems[2];
    HRESULT hr;
    VOID *dx_addr = NULL;
    DWORD dx_addr_size = 0;
    int elem_c = 0;

    // Service provider section
    addr_elems[elem_c].guidDataType = DPAID_ServiceProvider;
    addr_elems[elem_c].dwDataSize   = sizeof(GUID);
    addr_elems[elem_c].lpData       = (LPVOID)&DPSPGUID_TCPIP;  // TCP ID
    elem_c++;

    // TCP/IP section
    addr_elems[elem_c].guidDataType = DPAID_INet;
    addr_elems[elem_c].dwDataSize   = host_addr.size();
    // lpData is not const, but surely this won't modify the data
    // it points to... right? Surely hope so.
    addr_elems[elem_c].lpData       = (void *)host_addr.data();
    elem_c++;

    while (1) {
        // First iteration gets the needed size, second iteration
        // makes the allocation
        hr = _lobby->CreateCompoundAddress(
                 addr_elems, elem_c, dx_addr, &dx_addr_size
             );

        if (hr == DPERR_BUFFERTOOSMALL && !dx_addr) {
            ACE_DEBUG((LM_DEBUG, "launcher_dx7:_create_dx_addr allocating "
                       "buffer of size %d\n", dx_addr_size));
            dx_addr = GlobalAllocPtr(GHND, dx_addr_size);
        } else if (FAILED(hr)) {
            if (dx_addr) GlobalFreePtr(dx_addr);
            throw exceptionf(0, "launcher_dx7::_create_dx_addr failed at "
                             "CreateAddress(%d), addr size %d",
                             hr, dx_addr_size);
        } else {
            break;
        }
    }

    *dx_ret_addr = dx_addr;
    *dx_ret_size = dx_addr_size;
}

// Doesn't work
#if 0
void
launcher_dx7::_create_dx_addr(
    const std::string &host_addr,
    VOID **dx_ret_addr,
    DWORD *dx_ret_size)
{
    ACE_DEBUG((LM_DEBUG, "launcher_dx7::_create_dx_addr2 target pointers "
               "%d and %d\n", dx_ret_size, dx_ret_addr));

    VOID *dx_addr = NULL;
    DWORD dx_addr_size = 0;

    HRESULT hr = S_OK;
    while (1) {
        ACE_DEBUG((LM_DEBUG, "launcher_dx7::_create_dx_addr2 target pointers "
                   "%d and %d\n", dx_ret_size, dx_ret_addr));
        // First iteration gets the needed size, second iteration
        // makes the allocation
        hr = _lobby->CreateAddress(
                 DPAID_ServiceProvider, DPAID_INet,
                 host_addr.data(), host_addr.size(),
                 dx_addr, &dx_addr_size
             );
        ACE_DEBUG((LM_DEBUG, "launcher_dx7::_create_dx_addr2 target pointers "
                   "%d and %d\n", dx_ret_size, dx_ret_addr));

        if (hr == DPERR_BUFFERTOOSMALL && !dx_addr) {
            ACE_DEBUG((LM_DEBUG, "launcher_dx7:_run_app allocating "
                       "buffer of size %d\n", dx_addr_size));
            dx_addr = GlobalAllocPtr(GHND, dx_addr_size);
        } else if (FAILED(hr)) {
            if (dx_addr) GlobalFreePtr(dx_addr);
            throw exceptionf(0, "launcher_dx7::_run_app failed at "
                             "CreateAddress(%d), addr size %d",
                             hr, dx_addr_size);
        } else {
            break;
        }
    }

    ACE_DEBUG((LM_DEBUG, "launcher_dx7::_create_dx_addr2 setting "
               "address to %d size %d\n", dx_addr, dx_addr_size));
    ACE_DEBUG((LM_DEBUG, "launcher_dx7::_create_dx_addr2 target pointers "
               "%d and %d\n", dx_ret_size, dx_ret_addr));

    *dx_ret_size = dx_addr_size;
    ACE_DEBUG((LM_DEBUG, "launcher_dx7:: and then the address\n"));
    *dx_ret_addr = dx_addr;

    ACE_DEBUG((LM_DEBUG, "launcher_dx7::_create_dx_addr done\n"));
}
#endif

#if 0
int
launcher_dx7::_wstr_to_str(int dst_max_len, char *dst, const WCHAR *src)
{
    assert(dst != NULL);
    if (src == NULL) return 0;

    int src_len = wcslen(src);
    if (src_len + 1 > dst_max_len)
        throw exceptionf(0, "launcher_dx7::_wstr_to_str: buffer not big enough, "
                         "needed %d, max %d\n", src_len + 1, dst_max_len);

    WideCharToMultiByte(CP_ACP, 0, src, -1, dst, dst_max_len, NULL, NULL);
    dst[src_len] = 0;
    return src_len;
}
#endif

} // ns executable
