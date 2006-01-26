
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

//********************************************
//
// http_front::response_status implementation
//
//********************************************

void response_status::set_code(int code)
{
  assert(!m_status_set);
  if (!m_status_set)
  {
    m_code = code;

    switch (code)
    {
      case 200:
        m_message = "OK";
        break;
      case 302:
        m_message = "Moved Temporarily";
        break;
      case 303:
        m_message = "See Other";
        break;
      case 304:
        m_message = "Not Modified";
        break;
      case 400:
        m_message = "Bad Request";
        break;
      case 411:
        m_message = "Length Required";
        break;
      case 412:
        m_message = "Precondition Failed";
        break;
      case 501:
        m_message = "Not Implemented";
        break;
      case 505:
        m_message = "HTTP Version not supported";
        break;
      default:
        // code not handled
        assert(false);
    };

    m_status_set = true;
  }
}

//********************************************
//
// http_front::http_thread_pool implementation
//
//********************************************

http_front::http_thread_pool::http_thread_pool(wexus::core::front_peer_i* peer, int num_threads)
  : wexus::core::thread_pool<http_front::clientq_t>(num_threads), m_peer(peer)
{
  m_data.pm_data = 0;
}

pool_thread<http_front::clientq_t>* http_front::http_thread_pool::create_thread(void)
{
  return new wexus::core::http_thread(m_peer, get_data(), *this);
}

//********************************************
//
// http_front implementation
//
//********************************************

http_front::http_front(void)
  : thread(0), front_i(0), m_port(-1)
{
}

http_front::http_front(front_peer_i* peer, int thread_count, int port)
  : thread(0), front_i(peer), m_port(port)
{
  m_thread_pool = new http_thread_pool(peer, thread_count);
}

bool http_front::init_prop(front_peer_i *peer, scopira::tool::property *prop)
{
  set_peer(peer);

  int thread_count;
  const std::string* tc = prop->get_string_value("thread_count");
  if (!tc || !string_to_int(*tc, thread_count) || thread_count<1) {
    COREOUT << "Bad \"thread_count\" for http_front\n";
    return false;
  }

  const std::string* p = prop->get_string_value("port");
  if (!p || !string_to_int(*p, m_port) || m_port<1) {
    COREOUT << "Bad \"port\" for http_front\n";
    return false;
  }

  // create thread pool
  assert(m_thread_pool.get() == 0);
  m_thread_pool = new http_thread_pool(peer, thread_count);

  return true;
}

void http_front::run()
{
  netflow server(0, m_port);

  bool shutdown = false;

  if (server.failed()) {
    COREOUT << "http-front: ##### failed to bind to " << m_port << ", aborting... #####\n";
    exit(0);
  }

  COREOUT << "http-front: started [port=" << m_port << "]\n";

  // loop until front end shuts down
  while (!shutdown)
  {
    // loop until server accepts a client or front end starts shutting down
    while (!shutdown && !server.read_ready(1000))
      shutdown = !m_thread_pool->is_running();

    bool accepted = false;
    count_ptr<netflow> client = new netflow;

    if (!shutdown)
      accepted = server.accept(*client);

    if (!accepted)
      client = 0;  // delete client since no client was accepted
    else
    {
      // insert the client into the queue and notify a thread
      // to take over communication with the client
      locker_ptr<thread_data<http_front::clientq_t> > kptr(m_thread_pool->get_data());
      kptr->m_data.push_back(client);
      m_thread_pool->get_data().pm_condition.notify();
    }
  }

  COREOUT << "http-front: closing...\n";
}

/// starts the job
void http_front::start(void)
{
  // start the thread
  thread::start();

  // set shutdown to false
  m_thread_pool->start();
}

/// tells the job to try to stop
void http_front::notify_stop(void)
{
  m_thread_pool->notify_stop();
}

/// this blocks until the job stops
void http_front::wait_stop(void)
{
  m_thread_pool->wait_stop();

  thread::wait_stop();
}

//********************************************
//
// http_thread implementation
//
//********************************************

const std::string http_thread::m_http_version = "HTTP/1.0";

http_thread::http_thread(front_peer_i* peer, event_area<thread_data<http_front::clientq_t> >& data, http_front::http_thread_pool& pool)
  : wexus::core::pool_thread<wexus::core::http_front::clientq_t>(data), m_peer(peer), m_client(0)
  // ** temporary
  , m_pool(pool)
  // ** temporary
{
}

http_thread::~http_thread()
{
}

bool http_thread::parse_headers(std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last, headers_t& headers)
{
  // fill in header information
  std::vector<std::string> values;

  for (std::vector<std::string>::const_iterator it = first; it!=last+1; it++)
  {
    const char first_char = (*it)[0];
    std::string header;
    std::string data;

    if (first_char != ' ' && first_char != '\t')
    {
      // start of header
      if (!split_char(*it, ':', header, data))
        return false; // not a valid header
    }
    else
    {
      // continuation of header
      data = *it;
    }

    // lowercase header (storing and comparing header as lowercase)
    std::string lc_header;
    lowercase(header, lc_header);

    // parse the header
    if (!parse_header(lc_header, data, headers))
      return false;
  }

  return true;
}

bool http_thread::parse_header(const std::string& header, const std::string& data, headers_t& headers)
{
  std::vector<std::string> values;
  if (header == "if-modified-since" || header == "if-unmodified-since")
  {
    // push back date (leave comment(';') out if found
    values.push_back(data.substr(0, data.rfind(';')));
  }
  else
  {
    string_tokenize(data, values, ",");
  }

  int splits = values.size();

  // insert header info into headers type
  for (int j=0; j<splits; j++)
  {
    trim_left(values[j]);
    trim_right(values[j]);

    headers.insert(std::make_pair(header, values[j]));
  }

  return true;
}

void http_thread::get_request(request_t& request)
{
  assert(m_client.get());

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
  size_t body_write_size;    // size of buffer to write
  byte_t* body_buf_pos = 0;      // pos in buffer to start writing buffer data from
  size_t content_length = 0;
  bufferflow::byte_buffer_t buf;

  buf.resize(max_buf_size);
  message.reserve(max_buf_size);

  size_t read_size;
  bufferflow::byte_buffer_t::iterator dbl_crlf_pos;
  // get request line, headers and body (if one exists)
  // keep reading until a double crlf is found (body may exist after that though still)
  while ( (read_size = m_client->read_short(&buf[0], buf.size())) > 0) {
    assert(read_size>0);
    assert(read_size<=buf.size());
#ifndef NDEBUG
    //COREOUT.write(&buf[0], read_size);
#endif

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

      // decode request line (first line in request)
      if (!decode_request_line(lines[0], request)) {
        request.status.set_code(400); // bad request
        return;
      }
      
      // check HTTP version
      //if (request.version != http_version())
      //{
      //  request.status.set_code(505); // version not supported
      //  return;
      //}

      // parse header information from data
      // skip first line because it's the request line and
      // skip the last line because it's the CRLF
      if (!parse_headers(lines.begin()+1, lines.end()-1, request.headers)) {
        request.status.set_code(400); // bad request
        return;
      }

      // check if the host header was included in the HTTP1.1 request
      if (request.headers.find("host") == request.headers.end() && request.version == "HTTP/1.1") {
        // host header must be included in a 1.1 request
        request.status.set_code(400);
        return;
      }

      // headers have been read
      read_headers = true;

      // Lookup "Content-Length" header
      // if it exists then we have body data to read
      headers_t::iterator header = request.headers.find("content-length");
      if (header != request.headers.end()) {
        // get the content length
        content_length = string_to_int((*header).second);
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
      body_write_size = std::min(content_length-request.body.size(), read_size);
      body_buf_pos = &buf[0];
    }

    // lets append this portion to the body portion of our request
    if (read_headers && !read_body) {
      // check that we received a length
      // if not then set the status
      if (content_length == 0) {
        request.status.set_code(411); // no Content-Length header, but extra data exists
        return;
      }

      // write body data if any exists
      if (body_write_size != 0)
        request.body.write(body_buf_pos, body_write_size);

      // check if we've read all the body data
      if (request.body.size() == content_length)
        read_body = true;
    }
    
    // everything is done [todo simplify the checking here?]
    if (read_headers && read_body) {
      // body size must not be larger than the size specified by content length
      assert(request.body.size() <= content_length);

      // break if the body size matched the content length
      if (request.body.size() == content_length)
        break;
    }
  }//while read

  // parse request
  parse_request(request);
}

void http_thread::parse_request(request_t& request)
{
  // things to do for "GET" and "POST"
  if (request.method == "GET" || request.method == "POST") {
    // parse cookie values in the headers
    headers_t::iterator header = request.headers.find("cookie");
    if (header != request.headers.end())
      if (!request.client_cookies.decode_and_parse((*header).second)) {
        request.status.set_code(400);
        return;
      }
  }

  if (request.method == "GET") {
    // nothing for now
  } else if (request.method == "POST" && request.body.size() == 0) {
    // do nothing, this is ok
  } else if (request.method == "POST") {
    // retrieve buffer data and put it into a string
    flow_i::byte_t* buffer = request.body.c_array();
    std::string enc_data(reinterpret_cast<char*>(buffer), request.body.size());
    
    // decode form values
    if (!request.form_data.decode_and_parse(enc_data)) {
      request.status.set_code(400);
      return;
    }
  } else if (request.method == "HEAD") {
    // do nothing
  } else
    request.status.set_code(501); // requested method is not implemented
}

void http_thread::send_reply(request_t& request)
{
  assert(m_client.get());

  if (!m_peer) {
    COREOUT << "no peer exists\n";
    return;
  }

  bufferflow bf;
  event evt(bf, *m_client, request);

  // handle event if no code set
  if (request.status.get_code() == 0)
    m_peer->handle_front_event(evt);

  // send headers
  if (evt.send_headers())
    evt.send_body();
}

bool http_thread::decode_request_line(const std::string& request_line, request_t& request)
{
  // split initial line
  std::vector<std::string> values;
  string_tokenize_word(request_line, values, " ");

  // check that we have the correct number of fields
  if (values.size() == 3) {
    // fill in request line information

    // assign method
    request.method = values[0];

    // assign uri
    // check for form data
    std::string rawuri = values[1];
    std::string::size_type pos = rawuri.find('?');
    if (pos != std::string::npos) {
      // decode uri (left side of ?)
      core::url_decode(rawuri.substr(0, pos), request.uri, true);  

      // decode form data (right side of ?)
      if (pos != (rawuri.size()-1) &&
          !request.form_data.decode_and_parse(rawuri.substr(0+pos+1, std::string::npos)))
        return false;
    } else
      core::url_decode(rawuri, request.uri);

    // assign HTTP version
    request.version  = values[2];

    return true;
  }

  return false;
}

void http_thread::run()
{
  while (true) {
    {
      locker_ptr<thread_data<http_front::clientq_t> > lk_data(m_shared_data);

      // ** Temporary
      /*{
        locker_ptr<int> lk_count(m_pool.m_data);
        (*lk_count)++;
        COREOUT << "Idle Threads: " << (*lk_count) << '\n';
      }*/
      // ** Temporary

      // break out of loop if shutting down
      if (lk_data->m_shutdown)
        break;

      while (lk_data->m_data.empty() && !lk_data->m_shutdown)
        m_shared_data.pm_condition.wait(m_shared_data.pm_mutex);

      // ** Temporary
      /*{
        locker_ptr<int> lk_count(m_pool.m_data);
        (*lk_count)--;
        COREOUT << "Idle Threads: " << (*lk_count) << '\n';
      }*/
      // ** Temporary

      // break out of loop if shutting down
      if (lk_data->m_shutdown)
        break;

      m_client = lk_data->m_data.front();
      lk_data->m_data.pop_front();
    }//locker_ptr

    COREOUT << "http_front " << m_client->get_addr();

    request_t request;
    // get the request
    get_request(request);
    // send a reply
    send_reply(request);

    //COREOUT << "http-front: disconnected from " << m_client->get_addr() << '\n';

    m_client = 0;
  }//while
}

//********************************************
//
// http_thread::event implementation
//
//********************************************

http_thread::event::event(bufferflow& buffer, netflow& raw_buffer, request_t& request)
  : core::front_event_imp(&request.form_data, &request.client_cookies),
    m_buffer(buffer), m_raw_buffer(raw_buffer), m_request(request),
    m_sent_headers(false), m_sending_headers(false)
{
  i_set_request(request.uri);
}

oflow_i& http_thread::event::get_raw_output(void)
{
  // send the header info if it hasn't been sent already
  if (!(m_sent_headers || m_sending_headers))
    send_headers();

  return m_raw_buffer;
}

void http_thread::event::send_file(const std::string& name, size_t speed)
{
  size_t file_size = file(name).size();
  
  // set the content size
  set_content_size(file_size);

  // send the header info if it hasn't been sent already
  if (!m_sent_headers)
    send_headers();

#ifndef PLATFORM_win32
  /*
   * remove sendfile() support for now.
   * this may return if wexus ends up doing lots of static page serving
  if (speed == 0)
  {
    sendfile(
      m_raw_buffer.get_handle(),          // output handle
      open(name.c_str(), O_RDONLY, 0777), // input handle
      0,                                  // offset
      file_size);
  }
  else*/
#endif
  // use default send_file for windows and
  // for linux when using a non 0 speed
  front_event_imp::send_file(name, speed);
}

void http_thread::event::send_server_cookie(const std::string& cookie)
{
  oflow_i& client = get_raw_output();

  client << "Set-Cookie: " << cookie;
  client << "\r\n"; 
}

bool http_thread::event::check_headers(void)
{
  // check header info and set any corresponding status codes

  if (m_request.status.get_code() == 0) {
    if (!get_redirect_url().empty()) {
      // Reply status really should be 303 (I think), but
      // netscape won're redirect if the status code is 303.
      // Look into cause later.
      m_request.status.set_code(302);/*303*/;
      return false;
    }

    headers_t::iterator header;
    
    // check for "If-Modified-Since" header (only used with "GET")
    header = m_request.headers.find("if-modified-since");
    if (header != m_request.headers.end() && m_request.method == "GET") {
      if (get_modified_time() < httptime_to_timestamp((*header).second)) {
        // Not Modified
        m_request.status.set_code(304);
        return false;
      }
    }

    // check for "If-Unmodified-Since" header (used with any method)
    header = m_request.headers.find("if-unmodified-since");
    if (header != m_request.headers.end())
      if (get_modified_time() > httptime_to_timestamp((*header).second)) {
        m_request.status.set_code(412);
        return false;
      }
  }//get_code() == 0

  return true;
}

void http_thread::event::check_status(void)
{
  // create an error page if an error has been generated
  if (m_request.status.get_code() >= 400)
    create_error_page(
      m_request.status.get_code(),
      int_to_string(m_request.status.get_code()) + " " + m_request.status.get_message());

  // set status to 200(OK) if no errors have been generated
  if (m_request.status.get_code() == 0)
    m_request.status.set_code(200);
}

bool http_thread::event::send_headers()
{
  // Don't send header info if the headers have already been sent.
  // The headers can be sent if a front peer required raw access to
  // the output buffer.
  if (m_sent_headers)
    return false;

  m_sending_headers = true;

  bool send_body = check_headers();
  check_status();

  int status_code = m_request.status.get_code();
  oflow_i& client = m_raw_buffer;

  // send status line
  client <<
    http_version() << ' ' <<
    int_to_string(status_code) << ' ' <<
    m_request.status.get_message() << "\r\n";

  // all headers to be send to client

  // send "Server" header
  if (status_code != 304)
    client << "Server: Wexus/HTTP\r\n";

  std::string time;
  
  // send "Date" header (only codes 200 and up require the date header)
  if (status_code >= 200) {
    time = timestamp_to_httptime(timestamp::current_time());
    client << "Date: " << time << "\r\n";
  }

  if (status_code != 304) {
    // send "Last-Modified" header
    //event.get_modified_time().http_format(time);
    //*m_client << "Last-Modified: " << time << "\r\n";

    // send "Location" header
    std::string redirect_url = get_redirect_url();
    if (!redirect_url.empty())
      client << "Location: " << redirect_url << "\r\n";
    
    // send "Set-Cookie" header
    send_server_cookies();

    // send content type
    std::string content_type = get_content_type();
    if (!content_type.empty())
      client << "Content-Type: " << content_type << "\r\n";

    // send content length if the buffer contains any data
    int body_size = get_content_size();
    // body_size could be > 0 and the headers sent if
    // the peer used get_raw_ouput or send_file
    if (body_size == 0 && !m_sent_headers)
      body_size = m_buffer.size();

    if (body_size > 0)
      client << "Content-Length: " << body_size << "\r\n";
  }
  
  client << "\r\n";

  // set header sent flag
  m_sent_headers = true;

  m_sending_headers = false;

  return send_body;
}

void http_thread::event::send_body()
{
  // send buffer
  if (m_buffer.size() > 0) {
    // reset buffer so that it can be read from
    m_buffer.reset();
    // send buffer
    m_raw_buffer << m_buffer;
  }  
}

