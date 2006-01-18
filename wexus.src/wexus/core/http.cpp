
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

#include <wexus/core/http.h>

#include <scopira/tool/platform.h>
#include <scopira/tool/util.h>

#include <assert.h>

#ifdef PLATFORM_UNIX
#include <ctype.h>
#endif

//BBtargets libwexus.so

using namespace wexus;

void core::url_decode(const std::string& encoded, std::string& decoded, bool form)
{
  // clear output string
  decoded.clear();

  std::string hexstr;

  for (std::string::const_iterator it = encoded.begin(); it != encoded.end(); it++)
  {
    switch (*it)
    {
    // Convert all + chars to space chars
    case '+':
      decoded += ' ';
      break;
      
    // Convert all %xy hex codes into ASCII chars
    case '%':
      
      // Copy the two bytes following the %
      hexstr.assign(it+1, it+3);
      
      // Skip over the hex (next iteration will advance enc one more position)
      it += 2;
      
      // Convert the hex to ASCII
      // Prevent user from altering URL delimiter sequence (& and =)
      if( form && ((hexstr.compare("26")==0) || (hexstr.compare("3D")==0)) )
      {
        decoded += '%';
        decoded += hexstr;
      }
      else
        decoded += static_cast<char>(strtol(hexstr.c_str(), 0, 16));

      break;
      
    // Make an exact copy of anything else
    default:
      decoded += *it;
      break;
    }
  }
}

void core::url_encode(const std::string& str, std::string& encoded, bool form)
{
  // clear output string
  encoded.clear();

  char buffer[256];
  for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
  {
    if (*it == ' ' && form)
    {
      // only convert a space to '+' if we're encoding form data
      encoded += '+';
    }
    else if (!::isalnum(*it) && !(*it == '.' && !form))
    { 
      // encode all non-alpha numeric chars except for a '.' if we're
      // encoding non-form data (urls).

      // hex codes begin with a %
      encoded += "%";
      // convert the ASCII char to hex code
#ifdef PLATFORM_win32
      // faster than sprintf
      _ltoa(static_cast<int>(*it), buffer, 16);
#else
      snprintf(buffer, 256, "%x", static_cast<int>(*it));
#endif
      encoded += buffer;
    }
    else
      encoded += *it;
  }
}

std::string core::url_encode(const std::string& str, bool form)
{
  std::string ret;

  ret.reserve(str.size()*2);

  url_encode(str, ret, form);

  return ret;
}

void core::html_quote_encode(const std::string& str, std::string& out)
{
  out.clear();
  for (std::string::size_type i=0; i<str.size(); ++i)
  {
    switch (str[i])
    {
        case '"': out += "&quot;"; break;
        case '&': out += "&amp;"; break;
        case '<': out += "&lt;"; break;
        case '>': out += "&gt;"; break;
        default: out += str[i];
    }
  }
}

const std::string core::html_quote_encode(const std::string &str)
{
  std::string out;

  out.reserve(str.size());

  for (std::string::size_type i=0; i<str.size(); ++i)
  {
    switch (str[i])
    {
        case '"': out += "&quot;"; break;
        case '&': out += "&amp;"; break;
        case '<': out += "&lt;"; break;
        case '>': out += "&gt;"; break;
        default: out += str[i];
    }
  }

  return out;
}

void core::html_quote_decode(const std::string& str, std::string& out)
{
  out.clear();

  std::string buf; // buffer for the encoded text

  for (std::string::size_type i=0; i<str.size(); ++i)
  {
    if (str[i] == '&')
      buf += str[i];
    else if (str[i] == ';')
    {
      buf += str[i];
      if (buf == "&quot;")
        out += '"';
      else if (buf == "&amp;")
        out += '&';
      else if (buf == "&lt;")
        out += '<';
      else if (buf == "&gt;")
        out += '>';
      else
        out += buf;

      buf.clear();
    }
    else if (!buf.empty())
      buf += str[i];
    else
      out += str[i];
  }

  out += buf;
}

//********************************************
//
// tool::cookies implementation
//
//********************************************

core::cookies::cookies(void)
{
}

bool core::cookies::decode_and_parse(const std::string& raw_cookie_str)
{
  std::vector<std::string> values;
  // split string into name/value sets using the "; " seperators
  scopira::tool::string_tokenize(raw_cookie_str, values, "; ");

  std::string decoded_val;
  std::string name, value;
  // add name/value pairs to data map
  for (size_t i=0; i<values.size(); i++)
  {
    // split string into name/value pairs using the "=" seperator
    scopira::tool::split_char(values[i], '=', name, value);

    // decode cookie before inserting into map
    core::url_decode(value, decoded_val);
    m_cookies.insert(std::make_pair(name, decoded_val));
  }

  return true;
}

bool core::cookies::has_cookie(const std::string& name) const
{
  return m_cookies.find(name) != m_cookies.end();
}

void core::cookies::get_cookies(const std::string& name, std::vector<std::string>& values) const
{
  assert(has_cookie(name));

  values.clear();

  cookies_t::const_iterator it;
  for (it=m_cookies.lower_bound(name); it!=m_cookies.upper_bound(name); it++)
  {
    values.push_back((*it).second);
  }
}

//********************************************
//
// core::form_data implementation
//
//********************************************

core::form_data::form_data(void)
{
}

bool core::form_data::has_field(const std::string& name) const
{
  return m_form_data.find(name) != m_form_data.end();
}

const std::string& core::form_data::get_field(const std::string& name) const
{
  // name must exist
  assert(has_field(name));

  return (*m_form_data.find(name)).second;
}

bool core::form_data::decode_and_parse(const std::string& encoded)
{
  std::string hexstr;

  // currently iterating over the name?
  bool is_name = true;

  std::string name;
  std::string value;

  std::string::const_iterator it;
  for (it = encoded.begin(); it != encoded.end(); it++)
  {
    switch (*it)
    {
    // convert all + chars to space chars
    case '+':
      (is_name ? name : value) += ' ';
      break;
      
    // convert all %xy hex codes into ASCII chars
    case '%':

      // copy the two bytes following the %
      hexstr.assign(it+1, it+3);
      
      // skip over the hex (next iteration will advance enc one more position)
      it += 2;
      
      // convert the hex to ASCII
      (is_name ? name : value) += static_cast<char>(strtol(hexstr.c_str(), 0, 16));
      break;

    case '=':
      // begin parsing the value
       is_name = false;
       break;

    case '&':
      // begin parsing a new name
      is_name = true;
      // insert stored name and value into map
      m_form_data.insert(std::make_pair(name, value));
      // clear name/value strings for next pair
      name.clear();
      value.clear();
      break;

    // make an exact copy of anything else
    default:
      (is_name ? name : value) += *it;
      break;
    } 
  }

  if (!is_name)
  {
    // value parsed, insert name and value into map
    m_form_data.insert(std::make_pair(name, value));
    return true;
  }
  else
  {
    // currently parsing a name
    // error in URL syntax, no final value found
    return false;
  }
}

scopira::tool::timestamp wexus::core::httptime_to_timestamp(const std::string &http_format_time)
{
  char month[4], timezone[4];
  int day, year, hour, minute, second;

  // Parse all http formats
  // The first is the standard format, but the other two should be accepted since they
  // may be generated by older clients.
  if (sscanf(http_format_time.c_str(), "%*[^,], %d %s %d %d:%d:%d %s", &day, month, &year, &hour, &minute, &second, timezone) == 7)
  {
    // only GMT time is supported with this format.  Other zones will give incorrect times.
    assert(std::string(timezone) == "GMT");

    // Ex. Fri, 31 Dec 1999 23:59:59 GMT
    return scopira::tool::timestamp(year, scopira::tool::timestamp::string_to_month(month), day, hour, minute, second, -1);//, true);
  }
  else if (sscanf(http_format_time.c_str(), "%*[^,], %2d-%[^-]-%2d %d:%d:%d %s", &day, month, &year, &hour, &minute, &second, timezone) == 7)
  {
    // only GMT time is supported with this format.  Other zones will give incorrect times.
    assert(std::string(timezone) == "GMT");

    // Ex. Friday, 31-Dec-99 23:59:59 GMT
    // resolve any Y2K issues
    // there better not be any files from 1980 or before :)
    if (year < 80)
      year += 2000;
    else
      year += 1900;

    return scopira::tool::timestamp(year, scopira::tool::timestamp::string_to_month(month), day, hour, minute, second, -1);//, true);
  }
  else if (sscanf(http_format_time.c_str(), "%*s %s %d %d:%d:%d %d", month, &day, &hour, &minute, &second, &year) == 6)
  {
    // Ex. Fri Dec 31 23:59:59 1999
    return scopira::tool::timestamp(year, scopira::tool::timestamp::string_to_month(month), day, hour, minute, second);
  }

  return scopira::tool::timestamp();
}


std::string wexus::core::timestamp_to_httptime(scopira::tool::timestamp t)
{
  std::string ret;
  time_t yy = t.get_time();

  ret.resize(100);

  // dm_time must be valid
  assert(!t.is_null());

  // write http format to time var
#ifdef PLATFORM_win32
  int size = strftime(&ret[0], ret.size(), "%a, %d-%m-%Y %H:%M:%S GMT", gmtime(&yy));
#else
  struct tm timmy;
  gmtime_r(&yy, &timmy);
  int size = strftime(&ret[0], ret.size(), "%a, %d-%m-%Y %H:%M:%S GMT", &timmy);
#endif

  // resize to proper size
  ret.resize(size);

  return ret;
}
