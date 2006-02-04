
/*
 *  Copyright (c) 2004    Aleksander B. Demko
 *
 *  All rights reserved.
 *
 *  This material is confidential and proprietary information of
 *  Aleksander B. Demko ("Confidential Information").
 *  This Confidential Information may only be used and reproduced
 *  in accordance with the terms of the license agreement.
 *
 */

#include <assert.h>

#include <scopira/tool/platform.h>

// DISABLED FOR NOW
// #ifndef PLATFORM_win32
// #include <sys/sendfile.h>
// #endif

#include <scopira/tool/netflow.h>
#include <scopira/tool/fileflow.h>
#include <scopira/tool/prop.h>
#include <scopira/tool/file.h>
#include <scopira/tool/util.h>

#include <wexus/core/output.h>
#include <wexus/core/httpfront.h>

#include <algorithm>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus::core;

//
//
// http_front
//
//

http_front::http_front(void)
  : front_i(0), thread(0), m_port(-1)
{
  m_clientarea.pm_data.alive = true;
}

http_front::http_front(front_peer_i* peer, int thread_count, int port)
  : front_i(peer), thread(0), m_port(port)
{
  m_clientarea.pm_data.alive = true;

  assert(peer);

  assert(thread_count>0);

  for (int x=0; x<thread_count; ++x)
    m_workers.push_back(new http_worker(this, x));
}

bool http_front::init_prop(front_peer_i *peer, scopira::tool::property *prop)
{
  set_peer(peer);

  int thread_count;
  const std::string* tc = prop->get_string_value("thread_count");
  if (!tc || !string_to_int(*tc, thread_count) || thread_count<1) {
    OUTPUT << "Bad \"thread_count\" for http_front\n";
    return false;
  }

  const std::string* p = prop->get_string_value("port");
  if (!p || !string_to_int(*p, m_port) || m_port<1) {
    OUTPUT << "Bad \"port\" for http_front\n";
    return false;
  }

  // create thread pool
  for (int x=0; x<thread_count; ++x)
    m_workers.push_back(new http_worker(this, x));

  return true;
}

void http_front::run(void)
{
  netflow listenport(0, m_port);
  bool alive = true;
  count_ptr<netflow> client;

  if (listenport.failed()) {
    OUTPUT << "http_front failed_to_bind port=" << m_port << " full_exit\n";
    exit(0);
  }

  OUTPUT << "http_front started port=" << m_port << '\n';

  // loop until front end shuts down
  while (true) {
    // loop until listenport accepts a client or front end starts shutting down
    while (alive && !listenport.read_ready(2000)) {
      locker_ptr<client_area> L(m_clientarea);

      if (!L->alive)
        alive = false;
    }

    if (!alive)
      break;

    count_ptr<netflow> client = new netflow;

    if (listenport.accept(*client)) {
      // insert the client into the queue and notify a thread
      // to take over communication with the client
      {
        event_ptr<client_area> L(m_clientarea);

        // extreme flow control
        // if there are 2x more pending clients that workers, wait.
        while (L->clients.size() > 2*m_workers.size() && L->alive)
          L.wait(1000);
        if (!L->alive)
          break;

        // ok, push it to the pool so a worker thread takes it and processes it
        L->clients.push_back(client);
      }
      m_clientarea.pm_condition.notify();
    }
    // reuse client if the accept failed? hmmm
    client = 0;
  }

  OUTPUT << "http_front stopping\n";
}

void http_front::start(void)
{
  thread::start();

  for (int x=0; x<m_workers.size(); ++x)
    m_workers[x]->start();
}

void http_front::notify_stop(void)
{
  {
    locker_ptr<client_area> L(m_clientarea);

    L->alive = false;
  }
  m_clientarea.pm_condition.notify_all();
}

void http_front::wait_stop(void)
{
  for (int x=0; x<m_workers.size(); ++x)
    m_workers[x]->wait_stop();

  thread::wait_stop();
}

//
//
// http_worker
//
//

http_worker::http_worker(http_front *master, int myid)
  : thread(0), m_master(master), m_myid(myid)
{
  assert(master);
  assert(myid>=0);
}

void http_worker::run(void)
{
  count_ptr<netflow> clientflow;

  while (true) {
    {
      event_ptr<http_front::client_area> L(m_master->m_clientarea);

      if (!L->alive)
        return;

      while (L->alive && L->clients.empty())
        L.wait();

      if (!L->alive)
        return;

      clientflow = L->clients.front();
      L->clients.pop_front();
    }//locker_ptr

    // notify master/othre threads that the shared area has had one client processes
    m_master->m_clientarea.pm_condition.notify();

    assert(clientflow.get());

    OUTPUT << "http_front " << clientflow->get_addr();

    // make the event and process it based on the client's http request
    int code = process_event(*clientflow);

    OUTPUT << ' ' << code << '\n';

    clientflow = 0;
  }//while
}

int http_worker::process_event(scopira::tool::netflow &clientflow)
{
  http_event evt(clientflow);

  // declare array of chars
  // can't be string since it would have a null character at the end
  flow_i::byte_t  dbl_crlf[] = {'\r','\n','\r','\n'};
  size_t dbl_crlf_size = sizeof(dbl_crlf)/sizeof(dbl_crlf[0]);

  const int max_buf_size = 1024*4;
   // check size in case we're testing with a really small buffer
  assert(max_buf_size > dbl_crlf_size);

  bufferflow::byte_buffer_t message; // data read in
  bool read_headers = false; // are the headers read?
  bool read_body = false;    // have we received all of the body?
  int body_write_size;    // size of buffer to write
  byte_t* body_buf_pos = 0;      // pos in buffer to start writing buffer data from
  int content_length = 0;
  bufferflow::byte_buffer_t buf;

  buf.resize(max_buf_size);
  message.reserve(max_buf_size);

  size_t read_size;
  bufferflow::byte_buffer_t::iterator dbl_crlf_pos;
  // get request line, headers and body (if one exists)
  // keep reading until a double crlf is found (body may exist after that though still)
  while ( (read_size = clientflow.read_short(&buf[0], buf.size())) > 0) {
    assert(read_size>0);
    assert(read_size<=buf.size());

    // if we havent completed reading ourheaders, lets append this block to the message block
    // and check it for the important double-crlf marker
    if (!read_headers) {
      // start searching for the double crlf at the beginning if this is the
      // first time searching or from the end-3 (first 3 chars of the double
      // crlf might already exist in the message).
      // (basically, cover the case when the dbl spans two bufs)
      size_t msg_size = message.size();
      size_t search_pos = (msg_size <= dbl_crlf_size-1 ? 0 : msg_size-(dbl_crlf_size-1));

      // append buffer to message
      message.insert(message.end(), buf.begin(), buf.begin() + read_size);

      // search for the double crlf
      dbl_crlf_pos = std::search(message.begin()+search_pos, message.end(), dbl_crlf, dbl_crlf+dbl_crlf_size);
    }

    // since we dont have our headers, have we found it (in the previous block?)
    // if so, parse it
    if (!read_headers && dbl_crlf_pos != message.end()) {
      // split into request line and headers
      std::string txt_data(message.begin(), dbl_crlf_pos);
      std::vector<std::string> lines;
      string_tokenize_word(txt_data, lines, "\r\n");
      http_headers::headers_t::iterator ii;

      // decode request line (first line in request)
      if (!decode_request_line(lines[0], evt)) {
        evt.pm_status.set_code(400); // bad request
        break;
      }
      
      // parse header information from data
      // skip first line because it's the request line and
      // skip the last line because it's the CRLF
      if (!evt.pm_headers.parse_headers(lines.begin()+1, lines.end()-1)) {
        evt.pm_status.set_code(400); // bad request
        break;
      }

      // check if the host header was included in the HTTP1.1 request
      if (evt.pm_headers.pm_headers.find("host") == evt.pm_headers.pm_headers.end() && evt.pm_version == "HTTP/1.1") {
        // host header must be included in a 1.1 request
        evt.pm_status.set_code(400);
        break;
      }

      // check for X-Forwarded-For: IP.IP.IP.IP
      // which apache feeds us when we're in proxy mode
      // (could these be a security risk in non-proxy mode then?
      // Here is a sample of some more fields
      //  X-Forwarded-For: IP.IP.IP.IP
      //  X-Forwarded-Host: domain.com
      //  X-Forwarded-Server: domain.com
      if ( (ii = evt.pm_headers.pm_headers.find("x-forwarded-for")) != evt.pm_headers.pm_headers.end()
          && evt.pm_client_addr.parse_string(ii->second))
          OUTPUT << " " << ii->second; // parse the actual IP and store it in the event's m_client_addr

      /*ii = evt.pm_headers.pm_headers.find("content-type");
      if (ii != evt.pm_headers.pm_headers.end()) {
        // we have a content-type?
        OUTPUT << "CT:" << ii->second << '\n';
      }*/

      // headers have been read
      read_headers = true;

      // Lookup "Content-Length" header
      // if it exists then we have body data to read
      ii = evt.pm_headers.pm_headers.find("content-length");
      if (ii != evt.pm_headers.pm_headers.end()) {
        // get the content length
        if (string_to_int(ii->second, content_length))
          read_body = content_length == 0;      // if the content length is 0, lets skip the body outright
      } else
        read_body = true; // set to true since we have none to read anyway

      // any extra data is part of the body
      int msg_size = message.size();
      if (static_cast<int>(msg_size) > (dbl_crlf_pos+dbl_crlf_size)-message.begin()) {
        // size of buffer left to write
        body_write_size = msg_size-((dbl_crlf_pos+dbl_crlf_size)-message.begin());
        // content length and the amount read from the socket may differ,
        // write the smaller of the two sizes
        body_write_size = std::min(content_length, body_write_size);
        body_buf_pos = &(*dbl_crlf_pos)+dbl_crlf_size;
      } else
        body_write_size = 0;
    } else if (read_headers) {
      // ok, we've read the headers in a previous loop
      // lets mark this whole network byte block as writable to the body part of our message

      // content length and the amount read from the socket may differ,
      // write the smaller of the two sizes
      body_write_size = std::min(content_length-evt.pm_body.size(), read_size);
      body_buf_pos = &buf[0];
    }

    // lets append this portion to the body portion of our request
    if (read_headers && !read_body) {
      // check that we received a length
      // if not then set the status
      if (content_length == 0) {
        evt.pm_status.set_code(411); // no Content-Length header, but extra data exists
        break;
      }

      // write body data if any exists
      if (body_write_size != 0)
        evt.pm_body.write(body_buf_pos, body_write_size);

      // check if we've read all the body data
      if (evt.pm_body.size() == content_length)
        read_body = true;
    }
    
    // everything is done [todo simplify the checking here?]
    if (read_headers && read_body) {
      // body size must not be larger than the size specified by content length
      assert(evt.pm_body.size() <= content_length);

      // break if the body size matched the content length
      if (evt.pm_body.size() == content_length)
        break;
    }
  }//while read

  // parse evt, if ok
  if (evt.pm_status.get_code() == 0)
    parse_request(evt);

  // send it to the engine
  send_to_engine(evt);

  return evt.pm_status.get_code();
}

void http_worker::parse_request(http_event& evt)
{
  bool isget = evt.pm_method == "GET";
  bool ispost = evt.pm_method == "POST";

  // things to do for "GET" and "POST"
  if (isget || ispost) {
    // parse cookie values in the headers
    http_headers::headers_t::iterator header = evt.pm_headers.pm_headers.find("cookie");
    if (header != evt.pm_headers.pm_headers.end())
      if (!evt.pm_client_cookies.decode_and_parse((*header).second)) {
        evt.pm_status.set_code(400);
        return;
      }
  }

  if (isget) {
    // nothing for now
  } else if (ispost && evt.pm_body.size() == 0) {
    // do nothing, this is ok (post but with no content)
  } else if (ispost) {
    // retrieve buffer data and put it into a string
    flow_i::byte_t* buffer = evt.pm_body.c_array();
    std::string enc_data(reinterpret_cast<char*>(buffer), evt.pm_body.size());
    
    // decode form values
    if (!evt.pm_formdata.decode_and_parse(enc_data)) {
      evt.pm_status.set_code(400);
      return;
    }
  } else if (evt.pm_method == "HEAD") {
    // do nothing
  } else
    evt.pm_status.set_code(501); // requested method is not implemented
}

void http_worker::send_to_engine(http_event& evt)
{
  if (!m_master->get_peer()) {
    OUTPUT << "no peer exists\n";
    return;
  }

  // handle event if no code set
  if (evt.pm_status.get_code() == 0)
    m_master->get_peer()->handle_front_event(evt);

  // send headers
  if (evt.send_headers())
    evt.send_body();
}

bool http_worker::decode_request_line(const std::string& request_line, http_event& evt)
{
  // split initial line
  std::vector<std::string> values;
  string_tokenize_word(request_line, values, " ");

  // check that we have the correct number of fields
  if (values.size() == 3) {
    // fill in evt line information

    // assign method
    evt.pm_method = values[0];

    // assign uri
    // check for form data
    std::string rawuri = values[1];
    std::string::size_type pos = rawuri.find('?');
    if (pos != std::string::npos) {
      // decode uri (left side of ?)
      core::url_decode(rawuri.substr(0, pos), evt.pm_uri, true);  

      // decode form data (right side of ?)
      if (pos != (rawuri.size()-1) &&
          !evt.pm_formdata.decode_and_parse(rawuri.substr(0+pos+1, std::string::npos)))
        return false;
    } else
      core::url_decode(rawuri, evt.pm_uri);

    // assign HTTP version
    evt.pm_version = values[2];

    return true;
  }

  return false;
}

