#include <string>
#include <sstream>

#include "client.h"
#include "config.h"
#include "../../messaging/message.h"
#include "../../app_version.h"

#ifdef SEP
#undef SEP
#endif
#define SEP ';'

namespace networking {
namespace reporter {

client::client(ACE_Reactor *r) : _reactor(r)
{
    ACE_DEBUG((LM_DEBUG, "reporter::client ctor\n"));
    
    if (!_reactor) _reactor = ACE_Reactor::instance();
    ACE_INET_Addr server_addr(REPORTER_SERVER_PORT, REPORTER_SERVER_ADDR); 
    _handler = new handler(_reactor, this);

    _connector = 
        new ACE_Connector<handler, ACE_SOCK_CONNECTOR>(
            _reactor);

    ACE_Synch_Options conn_opts(ACE_Synch_Options::USE_REACTOR |
                                ACE_Synch_Options::USE_TIMEOUT,
                                ACE_Time_Value(30));
    ACE_OS::last_error(0);  
    if (_connector->connect(_handler, server_addr, conn_opts) == -1 &&
        ACE_OS::last_error() != EWOULDBLOCK)
    {
        ACE_ERROR((LM_ERROR, "%p\n", "reporter::client"));
        _handler->state = handler::disconnected;
    } else {
        _handler->state = handler::connecting;
    }
    
}

client::~client() {
    ACE_DEBUG((LM_DEBUG, "reporter::client dtor\n"));
    delete _connector;
    delete _handler;
}

void 
client::new_user(const chat_gaming::user &u, int grp) {
#if 0
    _send_user_report(u, grp);
#endif
}

void 
client::user_updated(const chat_gaming::user &oldu, 
                     const chat_gaming::user &newu, int grp)
{
    // Only send the first user update (presumably when user's login id
    // and other information has been obtained
    if (oldu.login_id().empty() && !newu.login_id().empty()) {
        // ui = user info
        _send_user_report("ui", newu, grp);
    }
}

void 
client::user_removed(const chat_gaming::user &u, int grp) {
    // ur = user removed
    _send_user_report("ur", u, grp);
}

void 
client::connected(int grp) {
    // cf = connect finished
    _send_connect_report("cf", grp);
}

void 
client::disconnected(int grp) {
    // dd = disconnected
    _send_connect_report("dd", grp);
}

void 
client::disabled(int grp) {
    // cf = connect disabled
    _send_connect_report("cd", grp);    
}

void
client::_send_user_report(const char *code, const chat_gaming::user &u, int grp) {
    if (!_handler) {
        ACE_DEBUG((LM_WARNING, "reporter::client::send_user_report "
                  " not connected, report not sent\n"));
        return;
    }
    std::ostringstream str;
    if (u.node()) {
        str << u.node()->addr().get_host_addr() << ":"
            << u.node()->addr().get_port_number()
            << SEP;
    } else {
        str << "NULL" << SEP;
    }
        
    str << APP_VERSION_ONLY
        << SEP << _self.id().id_str()
        << SEP << code
        << SEP << (grp == message::dht_group_base ? 'd' : 'c')
        << SEP << u.id().node()->addr().get_host_addr()
        << ":" << u.id().node()->addr().get_port_number()
        << SEP << u.id().id_str()
        << std::endl;
    
    ACE_DEBUG((LM_DEBUG, "%t reporter::client: sending report of size %d:\n%s", 
              str.str().size(), str.str().c_str()));
              
    _handler->send(str.str());
}

void
client::_send_connect_report(const char *code, int grp) {
    if (!_handler) {
        ACE_DEBUG((LM_WARNING, "reporter::client::send_connect_report "
                  " not connected, report not sent\n"));
        return;
    }

    std::ostringstream str;

    str << APP_VERSION_ONLY
        << SEP << _self.id().id_str()
        << SEP << code
        << SEP << (grp == message::dht_group_base ? 'd' : 'c')
        << std::endl;
    
    ACE_DEBUG((LM_DEBUG, "%t reporter::client: sending report of size %d:\n%s", 
              str.str().size(), str.str().c_str()));
              
    _handler->send(str.str());
}
   

/**********************************
 * HANDLER
 **********************************/
client::handler::handler(ACE_Reactor *r, client *owner)
    : super(NULL, NULL, r), 
      _owner(owner), 
      _notifier(NULL, this, ACE_Event_Handler::WRITE_MASK)
{}

client::handler::handler() : 
  _owner(NULL),
  _notifier(NULL, this, ACE_Event_Handler::WRITE_MASK)
  
{
    ACE_DEBUG((LM_DEBUG, "reporter::client::handler ctor should not happen!\n"));
}

client::handler::~handler() {
    ACE_DEBUG((LM_DEBUG, "reporter::client::handler dtor\n"));
    _release_blocks();
    _owner->_handler = NULL;
    // this->reactor()->remove_handler(this, ACE_Event_Handler::ALL_EVENTS_MASK);
}

void
client::handler::_release_blocks() {
    ACE_Message_Block *mb = NULL;
    ACE_Time_Value nowait(ACE_OS::gettimeofday());
    while (-1 != this->getq(mb, &nowait)) {
        ACE_DEBUG((LM_DEBUG, "client::handler releasing msg block %d\n", mb));
        mb->release();
    }   
}

void
client::handler::send(const std::string &str) {
    if (state != disconnected) {
        ACE_DEBUG((LM_DEBUG, " %t reporter::client:creating message block\n"));
        ACE_Message_Block *m = new ACE_Message_Block(str.size());
        ACE_DEBUG((LM_DEBUG, "reporter::client:copying the data\n"));
        m->copy(str.c_str(), str.size());
        ACE_DEBUG((LM_DEBUG, "reporter::client:inserting to queue\n"));
        // Sends notifies reactor automatically
        this->putq(m);
    }
}

int
client::handler::open(void *p) {
    ACE_DEBUG((LM_DEBUG, "%t reporter::client: connected to server, owner %d\n",
               _owner));
    if (super::open(p) == -1) {
        ACE_DEBUG((LM_DEBUG, "reporter::client: super returned -1\n"));
        return -1;
    }

    this->_notifier.reactor(this->reactor());
    // Setup notification so that each time we add something to the
    // queue handle_output is notified each time something is
    // added to the queue
    this->msg_queue()->notification_strategy(&this->_notifier);
    state = connected;
    
    if (!this->msg_queue ()->is_empty()) {
        ACE_DEBUG((LM_DEBUG, "reporter::client: reports queued, notify to write\n"));
        _notifier.notify();
    }
        
    return 0;
}

int 
client::handler::handle_input(ACE_HANDLE fd) {
    ACE_DEBUG((LM_DEBUG, "reporter::client: handle_input\n",
        _owner));
    char buf[64];
    // Basically just see if connection was closed
    ACE_OS::last_error(0);
    ssize_t size = this->peer().recv(buf, sizeof(buf) - 1);
    if (size <= 0 && ACE_OS::last_error() != EWOULDBLOCK) {
        ACE_DEBUG((LM_DEBUG, "reporter::client: read 0 bytes\n",
                   _owner));
        return -1;
    }
    return 0;
}

int 
client::handler::handle_output(ACE_HANDLE fd) {
    ACE_Message_Block *mb = NULL;
    ACE_Time_Value nowait(ACE_OS::gettimeofday());
    ACE_DEBUG((LM_DEBUG, "%t reporter::client:handle_output for %d\n", fd));
    if (state != connected) {
        ACE_DEBUG((LM_DEBUG, "reporter::client:handle_output skipped because not connected yet\n"));
        return 0;       
    }
    while (-1 != this->getq(mb, &nowait)) {
        ACE_DEBUG((LM_DEBUG, "reporter::client: sending data, mb: %d\n", mb));
        ssize_t send_cnt = this->peer().send(mb->rd_ptr(), mb->length());
        ACE_DEBUG((LM_DEBUG, "reporter::client: sent %d bytes\n", send_cnt));
        if (send_cnt == -1)
            ACE_ERROR((LM_ERROR, "(%P|%t) %p\n", "reporter_client::send"));
        else
            mb->rd_ptr (ACE_static_cast (size_t, send_cnt));
            
        if (mb->length() > 0) {
            this->ungetq (mb);
            break;
        }
        mb->release ();
    }

    if (this->msg_queue ()->is_empty()) {
        ACE_DEBUG((LM_DEBUG, "reporter::client: removing handler\n"));
        this->reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    } else {
        this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    }       
    return 0;   
}

int 
client::handler::handle_timeout(const ACE_Time_Value &tv, const void *p) {
    ACE_DEBUG((LM_DEBUG, "reporter::client::handler::handle_timeout\n"));
    return super::handle_timeout(tv, p);
}

int
client::handler::handle_close (ACE_HANDLE handle, ACE_Reactor_Mask close_mask) {
    ACE_DEBUG((LM_DEBUG, "reporter::client::handler::handle_close\n"));

    state = disconnected;
    
    return super::handle_close(handle, close_mask);
}


} // ns reporter
} // ns networking

