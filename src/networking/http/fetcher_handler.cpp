#include "fetcher_handler.h"

namespace networking {
namespace http {

fetcher_handler::fetcher_handler() {
    ACE_ERROR((LM_ERROR, "fetcher_handler default constructor called, should not happen!\n"));
}

fetcher_handler::fetcher_handler(const request &req, handler *h)
  : _handler(h)
{
    _recv_mb = new ACE_Message_Block(_max_response_size);

    // Create the message to send
    ACE_Message_Block *hdr_mb = new ACE_Message_Block(_max_header_size);
    
    // GET file HTTP/1.1
    // Connection: close
    //
    int n;
    n = ACE_OS::snprintf(hdr_mb->wr_ptr(), hdr_mb->space(), 
                         "%s %s HTTP/1.0\r\n",
                         req.method().c_str(), 
                         req.target_url().file().c_str());
    if (n > 0) hdr_mb->wr_ptr(n);
    n = ACE_OS::snprintf(hdr_mb->wr_ptr(), hdr_mb->space(), 
                         "Host: %s\r\n" \
                         "Connection: close\r\n\r\n",
                         req.target_url().host().c_str());
                         
    if (n > 0) hdr_mb->wr_ptr(n);
    if (hdr_mb->space()) {
        *(hdr_mb->wr_ptr()) = 0;
        ACE_DEBUG((LM_DEBUG, "Writing HTTP request\n",
                   hdr_mb->base()));
        // ACE_DEBUG((LM_DEBUG, "Writing HTTP request:\n%s",
        //           hdr_mb->base()));
    } else {
        throw exceptionf(0, "Not enough space in buffer of size %d for HTTP header",
                         _max_header_size);
    }
    
    this->putq(hdr_mb); 
}

fetcher_handler::~fetcher_handler() 
{
    delete _recv_mb;    
}

void 
fetcher_handler::fetch(const request &req, handler *h) {

}
           
// ACE_Event_Handler interface
int
fetcher_handler::open (void *p) {
    ACE_DEBUG((LM_DEBUG, "fetcher_handler::open\n"));
    
    // open is called when connected. The parent 
    // class's open registers us for read events.
    if (super::open(p) == -1) return -1;

    // Register for writing the request
    reactor()->register_handler(this, ACE_Event_Handler::WRITE_MASK);
        
    // Create 
    // TODO maybe register for a timeout?
    return 0;
}

int
fetcher_handler::handle_input (ACE_HANDLE fd) {
    char buf[_input_buffer_size];
    
    ACE_OS::last_error(0);
    ssize_t recv_cnt = this->peer().recv(buf, sizeof(buf) - 1);
    ACE_DEBUG((LM_DEBUG, "fetcher_handler: recv returned %d\n", recv_cnt));

    if (recv_cnt < 0 && ACE_OS::last_error () != EWOULDBLOCK) {
        ACE_ERROR((LM_ERROR, "fetcher_handler: %p\n", "recv"));
        _handler->handle_error(handler::err_socket, "recv failed");
        return -1;
    }
    if (recv_cnt == 0) {
        ACE_DEBUG((LM_DEBUG, "fetcher_handler: Close received\n"));
        _handle_response();
        return -1;
    }
    
    if (_recv_mb->copy(buf, recv_cnt)) {
        ACE_ERROR((LM_ERROR, "fetcher_handler: could not copy data to "
                  "receive buffer, space: %d, data len: %d\n",
                  _recv_mb->space(), recv_cnt));
        _handler->handle_error(handler::err_out_of_memory, "copying data failed");
        return -1;
    }
        
    return 0;
}

int 
fetcher_handler::handle_output(ACE_HANDLE fd) {
    ACE_Message_Block *mb = NULL;
    ACE_Time_Value nowait(ACE_OS::gettimeofday());
    ACE_DEBUG((LM_DEBUG, "fetcher_handler::handle_output for %d\n", fd));
    while (-1 != this->getq(mb, &nowait)) {
        ssize_t send_cnt = this->peer().send(mb->rd_ptr(), mb->length());
        ACE_DEBUG((LM_DEBUG, "fetcher_handler: sent %d bytes\n", send_cnt));
        if (send_cnt == -1)
            ACE_ERROR((LM_ERROR, "(%P|%t) %p\n", "fetcher_handler::send"));
        else
            mb->rd_ptr (ACE_static_cast (size_t, send_cnt));
        if (mb->length() > 0) {
            this->ungetq (mb);
            break;
        }
        mb->release ();
    }

    if (this->msg_queue ()->is_empty ())
        this->reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    else
        this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    return 0;   
}

int 
fetcher_handler::handle_timeout(const ACE_Time_Value &tv, const void *) {
    ACE_DEBUG((LM_DEBUG, "fetcher_handler::handle_timeout\n"));
    return 0;
}

int
fetcher_handler::handle_close (ACE_HANDLE handle, ACE_Reactor_Mask close_mask) {
    if (_recv_mb->length() <= 0) {
        // Assume a timeout if nothing read
        _handler->handle_error(http::handler::err_timeout, "Connect/read timeout");
    }
    _handler->handle_close();
    delete this;
    return 0;
}

int
fetcher_handler::_handle_response() {
    if (_recv_mb->space() <= 0) {
        _handler->handle_error(handler::err_out_of_memory, "no space for NULL");
        return -1;
    }
    
    // Try finding start of content from the message block
    // Terminate with NULL so that standard C string functions won't crash
    *(_recv_mb->wr_ptr()) = 0;

    const char *buf = _recv_mb->base();
    const char *cnt = NULL;
    if ((cnt = strstr(buf, "\r\n\r\n"))) {
        cnt += 4;
    } else if ((cnt = strstr(buf, "\n\n"))) {
        cnt += 2;
    } else {
        _handler->handle_error(handler::err_invalid_reply, "HTTP header delimiter not found");
        return -1;
    }
    
    // _recv_mb->rd_ptr(cnt);
    
    // Create a response object for reporting to handler
    response resp;
    resp.content    (cnt);
    resp.content_len((int)(_recv_mb->wr_ptr() - cnt));
    
    _handler->handle_response(resp);
    
    return 0;
}

} // ns http
} // ns networking

