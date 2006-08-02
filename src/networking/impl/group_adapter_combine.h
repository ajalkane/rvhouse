#ifndef _NETWORKING_GROUP_ADAPTER_COMBINE_H_
#define _NETWORKING_GROUP_ADAPTER_COMBINE_H_

#include <list>

#include "../group_adapter.h"
#include "../group_handler.h"

namespace networking {
class group_adapter_combine
            : public group_adapter
{
    typedef std::list<group_adapter *> _group_adapters_type;
    _group_adapters_type _gas; // List of group adapters
    std::list<int> _report_disabled;
public:
    group_adapter_combine(ACE_Reactor *r = NULL);
    virtual ~group_adapter_combine();

    virtual void init();
    virtual int  connect(const chat_gaming::user &self);
    virtual int  disconnect();
    virtual void refresh();
    virtual int  send(const std::string &msg, const std::string &channel, unsigned seq);
    virtual int  send_room(const std::string &msg,
                           const chat_gaming::room::id_type &rid, unsigned seq);

    virtual void update_server_nodes();

    virtual void update(const chat_gaming::user &r);
    virtual void update(const chat_gaming::room &r);
    virtual void join  (const chat_gaming::room &r, unsigned seq);
    virtual void join_rsp(const chat_gaming::user::id_type &uid, int rsp_code, unsigned seq);

    virtual void room_command(const chat_gaming::room::id_type &rid,
                              int command, unsigned seq);

    virtual void handle_message(message *msg);

};

} // ns networking

#endif //_NETWORKING_GROUP_ADAPTER_COMBINE_H_
