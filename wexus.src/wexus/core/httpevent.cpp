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

#include <wexus/core/httpevent.h>

#include <scopira/tool/flow.h>
#include <scopira/tool/fileflow.h>
#include <scopira/tool/file.h>
#include <scopira/tool/util.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus::core;

http_event::http_event(scopira::tool::netflow &_rawbuf)
  : pm_content_type("text/html"),
    pm_client_addr(_rawbuf.get_addr()),
    pm_raw_buffer(_rawbuf)
{
  pm_sent_headers = false;
  pm_sending_headers = false;
}

scopira::tool::oflow_i& http_event::get_raw_output(void)
{
  // send the header info if it hasn't been sent already
  if (!(pm_sent_headers || pm_sending_headers))
    send_headers();

  return pm_raw_buffer;
}

void http_event::set_content_type(const std::string& type)
{
  // content must always be all lower case
  lowercase(type, pm_content_type);
}

void http_event::send_file(const std::string& name, size_t speed)
{
  size_t file_size = file(name).size();
  
  // set the content size
  set_content_size(file_size);

  // send the header info if it hasn't been sent already
  if (!pm_sent_headers)
    send_headers();

  // the content type must be set
  assert(!pm_content_type.empty());

  // convert KB to bytes (easier for calculations)
  speed *= 1024;

  // set the content size
  set_content_size(file(name).size());

  fileflow file(name, fileflow::input_c);
  oflow_i& out = get_raw_output();

  const int buf_size = 1024*64; // default buffer size to read and write
  iflow_i::byte_t buf[buf_size];
  size_t sz;

  if (speed == 0)
    while ( (sz = file.read(buf, buf_size)) > 0 && !out.failed())
      if (out.write(buf, sz) == 0)
        break;
  else {
    const int sample_rate = 2; // number of times per second to sample the throttle
    size_t sample_size = 0;   // total data read for each sample
    
    timestamp t = timestamp::current_time(); // start timing
    while ( (sz = file.read(buf, buf_size)) > 0 && !out.failed()) {
      if (out.write(buf, sz) == 0)
        break;

      sample_size += sz;

      // calculate actual time in the sample period
      timespan actual_time = timestamp::current_time() - t;

      if (((sample_size >= speed*sample_rate) && (actual_time < sample_rate)) ||
          ((sample_size > speed*sample_rate) && (actual_time >= sample_rate))) {
        // time and data are out of whack with the throttle speed
        // calculate a sleep time
        size_t actual_speed;
        if (actual_time.get_total_seconds() == 0)
          actual_speed = sample_size;
        else
          actual_speed = sample_size/actual_time.get_total_seconds();

        if (actual_speed >= speed)
          thread::sleep(static_cast<int>((static_cast<double>(actual_speed)/static_cast<double>(speed))*1000.0));

        // reset timer and sample total
        t = timestamp::current_time();
        sample_size = 0;
      } else if ((sample_size <= speed*sample_rate) && (actual_time >= sample_rate)) {
        // everything is under control
        // just reset the timer and continue
        t = timestamp::current_time();
        sample_size = 0;
      }
    }
  }//else
}

void http_event::set_server_cookie(const std::string& name, const std::string& value,
  const std::string& path, timestamp expires, const std::string& domain)
{
  std::string expire_date;

  // can't use an empty name
  assert(!name.empty());

  if (expires != -1)
    expire_date = timestamp_to_httptime(expires); // get GMT version of 'expires' variable

  // fill in cookie information
  http_server_cookies::cookie_t cook;
  cook.name = name;
  core::url_encode(value, cook.value, true);  // url encode cookie data
  cook.expires = expire_date;
  cook.domain = domain;
  cook.path = path;

  // insert cookie with associated path and name
  pm_server_cookies.pm_cookies.push_back(cook);
}

bool http_event::has_client_cookie(const std::string& name) const
{
  return pm_client_cookies.has_cookie(name);
}

void http_event::get_client_cookies(const std::string& name, std::vector<std::string>& values) const
{
  pm_client_cookies.get_cookies(name, values);
}

bool http_event::has_form_field(const std::string& name) const
{
  return pm_formdata.has_field(name);
}

const std::string& http_event::get_form_field(const std::string& name) const
{
  assert(pm_formdata.has_field(name));

  return pm_formdata.get_field(name);
}

void http_event::create_error_page(int status, const std::string& message)
{
  // page is only necessary for error codes, which are 400 and above
  assert(status >= 400);

  if (status >= 400) {
    // customized error pages must be greater than 512 bytes in order for IE to not
    // display 'friendly' error page.  Can be any size for other browsers.
    // ##do something better with this in the future
    get_output() <<
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">"
      "<html>\n"
      "<head><title>" << message << "</title></head>\n"
      "<body bgcolor=\"#FFFFFF\" text=\"#000000\">\n"
      "<h2>ERROR " << message << "</h2>\n"
      "<p>                                                                  "
      "                                                                     "
      "                                                                     "
      "                                                                     "
      "                                                                     "
      "                                                                 </p>"
      "</body>\n"
      "</html>\n";
  }
}

bool http_event::check_headers(void)
{
  // check header info and set any corresponding status codes

  if (pm_status.get_code() == 0) {
    if (!pm_redirect_url.empty()) {
      // Reply status really should be 303 (I think), but
      // netscape won're redirect if the status code is 303.
      // Look into cause later.
      pm_status.set_code(302);/*303*/;
      return false;
    }

    http_headers::headers_t::iterator header;
    
    // check for "If-Modified-Since" header (only used with "GET")
    header = pm_headers.pm_headers.find("if-modified-since");
    if (header != pm_headers.pm_headers.end() && pm_method == "GET") {
      if (pm_modified_time < httptime_to_timestamp((*header).second)) {
        // Not Modified
        pm_status.set_code(304);
        return false;
      }
    }

    // check for "If-Unmodified-Since" header (used with any method)
    header = pm_headers.pm_headers.find("if-unmodified-since");
    if (header != pm_headers.pm_headers.end())
      if (pm_modified_time > httptime_to_timestamp((*header).second)) {
        pm_status.set_code(412);
        return false;
      }
  }//get_code() == 0

  return true;
}

void http_event::check_status(void)
{
  // create an error page if an error has been generated
  if (pm_status.get_code() >= 400)
    create_error_page(
      pm_status.get_code(),
      int_to_string(pm_status.get_code()) + " " + pm_status.get_message());

  // set status to 200(OK) if no errors have been generated
  if (pm_status.get_code() == 0)
    pm_status.set_code(200);
}

bool http_event::send_headers(void)
{
  // Don't send header info if the headers have already been sent.
  // The headers can be sent if a front peer required raw access to
  // the output buffer.
  if (pm_sent_headers)
    return false;

  pm_sending_headers = true;

  bool send_body = check_headers();
  check_status();

  int status_code = pm_status.get_code();
  oflow_i& client = pm_raw_buffer;

  // send status line
  client <<
    pm_version << ' ' <<
    int_to_string(status_code) << ' ' <<
    pm_status.get_message() << "\r\n";

  // all headers to be send to client

  // send "Server" header
  if (status_code != 304)
    client << "Server: Wexus/HTTP\r\n";     //policy: never encode any version number into this banner

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
    if (!pm_redirect_url.empty())
      client << "Location: " << pm_redirect_url << "\r\n";
    
    // send "Set-Cookie" header
    pm_server_cookies.send_server_cookies(client);

    // send content type
    if (!pm_content_type.empty())
      client << "Content-Type: " << pm_content_type << "\r\n";

    // send content length if the buffer contains any data
    int body_size = pm_content_size;
    // body_size could be > 0 and the headers sent if
    // the peer used get_raw_ouput or send_file
    if (body_size == 0 && !pm_sent_headers)
      body_size = pm_buffer.size();

    if (body_size > 0)
      client << "Content-Length: " << body_size << "\r\n";
  }
  
  client << "\r\n";

  // set header sent flag
  pm_sent_headers = true;

  pm_sending_headers = false;

  return send_body;
}

void http_event::send_body(void)
{
  // send buffer
  if (pm_buffer.size() > 0) {
    // reset buffer so that it can be read from
    pm_buffer.reset();
    // send buffer
    pm_raw_buffer << pm_buffer;
  }  
}

