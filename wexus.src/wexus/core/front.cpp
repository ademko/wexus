
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

#include <wexus/core/front.h>

#include <scopira/tool/util.h>
#include <scopira/tool/fileflow.h>
#include <scopira/tool/thread.h>
#include <scopira/tool/file.h>

#include <wexus/core/http.h>
#include <wexus/core/output.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus;
using namespace wexus::core;

front_event_imp::front_event_imp(const core::form_data* formdata, const core::cookies* cookies, const std::string& content_type)
  : m_content_type(content_type), m_content_size(0), m_formdata(formdata),
    m_client_cookies(cookies), m_modified_time(timestamp::current_time())
{
}

void front_event_imp::set_content_type(const std::string& type)
{
  // store content type as lower case
  lowercase(type, m_content_type);
}

#ifdef DEBUG_THROTTLE
void front_event_imp::display_speed(timestamp start_time, size_t sample_size, int sample_rate)
{
  timestamp current_time = timestamp::current_time();
  if (current_time >= start_time+sample_rate)
  {
    int debug_speed;
    if ((current_time - start_time).get_total_seconds() == 0)
      debug_speed = sample_size;
    else
      debug_speed = sample_size/(current_time - start_time).get_total_seconds();

    COREOUT << "Sending at ~" << debug_speed << " bytes/second\n";
  }
}
#endif

void front_event_imp::send_file(const std::string& name, size_t speed)
{
  // the content type must be set
  assert(!get_content_type().empty());

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
  {
#ifdef DEBUG_THROTTLE
    size_t sample_size = 0;
    timestamp start_time = timestamp::current_time();
    const int sample_rate = 2;
#endif
    while ( (sz = file.read(buf, buf_size)) > 0 && !out.failed())
    {
      if (out.write(buf, sz) == 0)
        break;

#ifdef DEBUG_THROTTLE
      sample_size += sz;
      if (timestamp::current_time() >= start_time+sample_rate)
      {
        display_speed(start_time, sample_size, sample_rate);
        start_time = timestamp::current_time();
        sample_size = 0;
      }
#endif
    }
  }
  else
  {
    const int sample_rate = 2; // number of times per second to sample the throttle
    size_t sample_size = 0;   // total data read for each sample
    
    timestamp t = timestamp::current_time(); // start timing
    while ( (sz = file.read(buf, buf_size)) > 0 && !out.failed())
    {
      if (out.write(buf, sz) == 0)
        break;

      sample_size += sz;

      // calculate actual time in the sample period
      timespan actual_time = timestamp::current_time() - t;

      if (((sample_size >= speed*sample_rate) && (actual_time < sample_rate)) ||
          ((sample_size > speed*sample_rate) && (actual_time >= sample_rate)))
      {
        // time and data are out of whack with the throttle speed
        // calculate a sleep time
        size_t actual_speed;
        if (actual_time.get_total_seconds() == 0)
          actual_speed = sample_size;
        else
          actual_speed = sample_size/actual_time.get_total_seconds();

        if (actual_speed >= speed)
          thread::sleep(static_cast<int>((static_cast<double>(actual_speed)/static_cast<double>(speed))*1000.0));

#ifdef DEBUG_THROTTLE
        display_speed(t, sample_size, sample_rate);
#endif
        // reset timer and sample total
        t = timestamp::current_time();
        sample_size = 0;
      }
      else if ((sample_size <= speed*sample_rate) && (actual_time >= sample_rate))
      {
#ifdef DEBUG_THROTTLE
        display_speed(t, sample_size, sample_rate);
#endif
        // everything is under control
        // just reset the timer and continue
        t = timestamp::current_time();
        sample_size = 0;
      }
    }
  }
}

void front_event_imp::set_server_cookie(const std::string& name, const std::string& value,
  const std::string& path, timestamp expires, const std::string& domain)
{
  std::string expire_date;

  // can't use an empty name
  assert(!name.empty());

  if (expires != -1)
    expire_date = timestamp_to_httptime(expires); // get GMT version of 'expires' variable

  // fill in cookie information
  cookie_data_t cookiedata;
  core::url_encode(value, cookiedata.value, true);  // url encode cookie data
  cookiedata.expires = expire_date;
  cookiedata.domain = domain;

  // insert cookie data with associated name
  cookiemap_t cookiemap;
  cookiemap.insert(std::make_pair(name, cookiedata));
  
  // insert cookie with associated path
  m_server_cookies.insert(std::make_pair(path, cookiemap));
}

bool front_event_imp::has_client_cookie(const std::string& name) const
{
  return (m_client_cookies ? m_client_cookies->has_cookie(name) : false);
}

void front_event_imp::get_client_cookies(const std::string& name, std::vector<std::string>& values) const
{
  if (m_client_cookies)
    m_client_cookies->get_cookies(name, values);
}

bool front_event_imp::has_form_field(const std::string& name) const
{
  return (m_formdata ? m_formdata->has_field(name) : false);
}

const std::string& front_event_imp::get_form_field(const std::string& name) const
{
  assert(m_formdata);
  assert(m_formdata->has_field(name));

  return (m_formdata->get_field(name));
}

