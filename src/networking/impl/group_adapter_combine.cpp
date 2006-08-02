#include "../../messaging/message.h"
#include "../worker.h"
#include "../global.h"
#include "../reporter/client.h"

#include "group_adapter_centralized.h"
#include "group_adapter_serverless.h"

#include "group_adapter_combine.h"

namespace networking {

group_adapter_combine::group_adapter_combine(
    ACE_Reactor *r
) : group_adapter(r)
{
    if (!net_conf()->get<bool>("net_centralized", "disable", false))
        _gas.push_back(new group_adapter_centralized(r));
    else
        _report_disabled.push_back(message::ctz_group_base);

    if (!net_conf()->get<bool>("net_serverless", "disable", false))
        _gas.push_back(new group_adapter_serverless (r));
    else
        _report_disabled.push_back(message::dht_group_base);
}

group_adapter_combine::~group_adapter_combine() {
    _group_adapters_type::iterator i = _gas.begin();
    for (; i != _gas.end(); i++)
        delete *i;
}

void group_adapter_combine::init() {
    _group_adapters_type::iterator i = _gas.begin();
    for (; i != _gas.end(); i++)
        (*i)->init();
}

int group_adapter_combine::connect(const chat_gaming::user &self) {
    net_report()->user_self(self);
    while (_report_disabled.size() > 0) {
        net_report()->disabled(_report_disabled.front());
        _report_disabled.pop_front();
    }

    _group_adapters_type::iterator i = _gas.begin();
    for (; i != _gas.end(); i++)
        (*i)->connect(self);

    return 0;
}


int group_adapter_combine::disconnect() {
    _group_adapters_type::iterator i = _gas.begin();
    for (; i != _gas.end(); i++)
        (*i)->disconnect();

    return 0;
}

void group_adapter_combine::refresh() {
    _group_adapters_type::iterator i = _gas.begin();
    for (; i != _gas.end(); i++)
        (*i)->refresh();
}

int
group_adapter_combine::send(const std::string &msg, const std::string &channel, unsigned seq) {
    _group_adapters_type::iterator i = _gas.begin();
    for (; i != _gas.end(); i++)
        (*i)->send(msg, channel, seq);
    return 0;
}

int
group_adapter_combine::send_room(const std::string &msg,
                                 const chat_gaming::room::id_type &rid,
                                 unsigned seq)
{
    _group_adapters_type::iterator i = _gas.begin();
    for (; i != _gas.end(); i++)
        (*i)->send_room(msg, rid, seq);
    return 0;

}

void
group_adapter_combine::update_server_nodes() {
    _group_adapters_type::iterator i = _gas.begin();
    for (; i != _gas.end(); i++)
        (*i)->update_server_nodes();
}

void
group_adapter_combine::update(const chat_gaming::user &u) {
    _group_adapters_type::iterator i = _gas.begin();
    for (; i != _gas.end(); i++)
        (*i)->update(u);
}

void
group_adapter_combine::update(const chat_gaming::room &r) {
    _group_adapters_type::iterator i = _gas.begin();
    for (; i != _gas.end(); i++)
        (*i)->update(r);
}

void
group_adapter_combine::join(const chat_gaming::room &r, unsigned seq) {
    _group_adapters_type::iterator i = _gas.begin();
    for (; i != _gas.end(); i++)
        (*i)->join(r,seq);
}

void
group_adapter_combine::join_rsp(const chat_gaming::user::id_type &r, int rsp, unsigned seq) {
    _group_adapters_type::iterator i = _gas.begin();
    for (; i != _gas.end(); i++)
        (*i)->join_rsp(r, rsp,seq);
}

void
group_adapter_combine::room_command(const chat_gaming::room::id_type &rid,
                                    int command,
                                    unsigned seq)
{
    _group_adapters_type::iterator i = _gas.begin();
    for (; i != _gas.end(); i++)
        (*i)->room_command(rid, command, seq);
}

void
group_adapter_combine::handle_message(message *msg) {
    _group_adapters_type::iterator i = _gas.begin();
    for (; i != _gas.end(); i++)
        (*i)->handle_message(msg);
}


} // ns networking
