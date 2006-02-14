
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
#include <scopira/tool/output.h>
#include <scopira/tool/hexflow.h>

#include <assert.h>

#ifdef PLATFORM_UNIX
#include <ctype.h>
#endif

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus::core;

void wexus::core::url_decode(const std::string& encoded, std::string& decoded, bool form)
{
  // clear output string
  decoded.clear();

  std::string hexstr;

  hexstr.reserve(4);

  for (std::string::const_iterator it = encoded.begin(); it != encoded.end(); it++)
    switch (*it) {
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
        if( form && ((hexstr.compare("26")==0) || (hexstr.compare("3D")==0)) ) {
          decoded += '%';
          decoded += hexstr;
        } else
          decoded += static_cast<char>(strtol(hexstr.c_str(), 0, 16));

        break;
        
      // Make an exact copy of anything else
      default:
        decoded += *it;
        break;
    }
}

void wexus::core::url_encode(const std::string& str, std::string& encoded, bool form)
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

std::string wexus::core::url_encode(const std::string& str, bool form)
{
  std::string ret;

  ret.reserve(str.size()*2);

  wexus::core::url_encode(str, ret, form);

  return ret;
}

void wexus::core::html_quote_encode(const std::string& str, std::string& out)
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

const std::string wexus::core::html_quote_encode(const std::string &str)
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

void wexus::core::html_quote_decode(const std::string& str, std::string& out)
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

//
//
// http_status
//
//

http_status::http_status(void)
  : m_code(0), m_message(0)
{
}

void http_status::set_code(int code)
{
  if (m_message)
    return;

  m_code = code;

  switch (code) {
    case 200: m_message = "OK"; break;
    case 302: m_message = "Moved Temporarily"; break;
    case 303: m_message = "See Other"; break;
    case 304: m_message = "Not Modified"; break;
    case 400: m_message = "Bad Request"; break;
    case 411: m_message = "Length Required"; break;
    case 412: m_message = "Precondition Failed"; break;
    case 500: m_message = "Server Error"; break; // used when postmax is exceeeded
    case 501: m_message = "Not Implemented"; break;
    case 505: m_message = "HTTP Version not supported"; break;
    default: m_message = "Unkown Code"; // code not handled
  };
}

//
//
// http_headers
//
//

bool http_headers::parse_headers(std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last)
{
  // fill in header information
  std::vector<std::string> values;
  std::string header;
  std::string data;

  header.reserve(64);
  data.reserve(64);

  for (; first!=last+1; first++) {
    if (first->empty())
      continue;
    const char first_char = (*first)[0];
    header.clear();
    data.clear();

    if (first_char != ' ' && first_char != '\t') {
      // start of header
      if (!split_char(*first, ':', header, data))
        return false; // not a valid header
    } else
      data = *first; // continuation of header

    // lowercase header (storing and comparing header as lowercase)
    std::string lc_header;
    lowercase(header, lc_header);

    // parse the header
    if (!parse_header(lc_header, data))
      return false;
  }

  return true;
}

bool http_headers::parse_header(const std::string& header, const std::string& data)
{
  std::vector<std::string> values;
  if (header == "if-modified-since" || header == "if-unmodified-since")
    values.push_back(data.substr(0, data.rfind(';'))); // push back date (leave comment(';') out if found
  else
    string_tokenize(data, values, ",");

  int splits = values.size();

  // insert header info into headers type
  for (int j=0; j<splits; j++) {
    trim_left(values[j]);
    trim_right(values[j]);

    pm_headers.insert(std::make_pair(header, values[j]));
  }

  return true;
}

//
//
// cookies
//
//

http_cookies::http_cookies(void)
{
}

bool http_cookies::decode_and_parse(const std::string& raw_cookie_str)
{
  std::vector<std::string> values;
  // split string into name/value sets using the "; " seperators
  string_tokenize_word(raw_cookie_str, values, "; ");

  std::string decoded_val;
  std::string name, value;

  decoded_val.reserve(64);
  name.reserve(64);
  value.reserve(64);

  // add name/value pairs to data map
  for (size_t i=0; i<values.size(); i++) {
    // split string into name/value pairs using the "=" seperator
    split_char(values[i], '=', name, value);

    // decode cookie before inserting into map
    url_decode(value, decoded_val);
    m_cookies.insert(std::make_pair(name, decoded_val));
  }

  return true;
}

bool http_cookies::has_cookie(const std::string& name) const
{
  return m_cookies.find(name) != m_cookies.end();
}

void http_cookies::get_cookies(const std::string& name, std::vector<std::string>& values) const
{
  assert(has_cookie(name));

  values.clear();

  cookies_t::const_iterator it;
  for (it=m_cookies.lower_bound(name); it!=m_cookies.upper_bound(name); it++)
  {
    values.push_back((*it).second);
  }
}

//
//
// http_event
//
//

void http_server_cookies::send_server_cookies(scopira::tool::oflow_i &client) const
{
  cookies_t::const_iterator ii, endii;

  ii = pm_cookies.begin();
  endii = pm_cookies.end();
  std::string cookie_hdr;

  cookie_hdr.reserve(128);

  for (; ii != endii; ++ii) {
    cookie_hdr = "Set-Cookie: ";
    cookie_hdr += ii->name + "=" + ii->value + "; path=" + ii->path;

    if (!ii->domain.empty())
      cookie_hdr += "; domain=" + ii->domain;

    if (!ii->expires.empty())
      cookie_hdr += "; expires=" + ii->expires;

    cookie_hdr += "\r\n"; 

    // finally, send it out over the socket
    client << cookie_hdr;
  }
}

//
//
// http_form
//
//

http_form::http_form(void)
{
}

bool http_form::has_field(const std::string& name) const
{
  return m_form_data.find(name) != m_form_data.end();
}

const std::string& http_form::get_field(const std::string& name) const
{
  // name must exist
  assert(has_field(name));

  return (*m_form_data.find(name)).second;
}

bool http_form::decode_and_parse(const char *encoded_begin, const char *encoded_end)
{
  //print_binary(encoded.c_str(), encoded.size());
  std::string hexstr;

  // currently iterating over the name?
  bool is_name = true;

  std::string name;
  std::string value;

  name.reserve(64);
  value.reserve(64);

  const char * it;
  for (it = encoded_begin; it != encoded_end; it++) {
    switch (*it) {
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

  if (!is_name) {
    // value parsed, insert name and value into map
    m_form_data.insert(std::make_pair(name, value));
    return true;
  } else {
    // currently parsing a name
    // error in URL syntax, no final value found
    return false;
  }
}

static void trim_quote(std::string &str)
{
  if (str.size()>1 && str[0] == '"' && str[str.size()-1] == '"')
    str = str.substr(1, str.size()-2);
}

// this turns form[fielname] into form[fielname.membername]
// ie. this kind of makes http_front aware of turbo-esque things... question, yeah
static std::string member_field_name(const std::string &fieldname, const std::string &membername)
{
  if (fieldname.empty())
    return membername;
  if (fieldname[fieldname.size()-1] == ']')
    return fieldname.substr(0, fieldname.size()-1) + "_" + membername + "]";
  return fieldname + "_" + membername;
}

bool http_form::decode_and_parse_upload(const char *encoded_begin, const char *encoded_end, const std::string &boundrystring)
{
//print_binary(encoded_begin, encoded_end - encoded_begin);
  std::string line;
  std::string field_name, field_filename, field_contenttype;
  const char *blockstart, *blockend, *cur, *anc, *mid;

  line.reserve(128);

  cur = encoded_begin;

  // find first blockstart
  blockstart = 0;
  ++cur;
  for (; blockstart == 0; ++cur) {
    if (cur == encoded_end)
      return false;
    if (*cur == '-' && *(cur - 1) == '-' && cur+3+boundrystring.size()<encoded_end &&   //+3 includes the \r\n
        std::string(cur+1, boundrystring.size()) == boundrystring)
      blockstart = cur + 3 + boundrystring.size();    // found it +2 is next+\r\n
  }

  // this loop processes each blockstart - blockend piece
  blockend = 0;
  while (true) {
    // assumes blockstart is always good
    assert(blockstart < encoded_end);

    // find the block end so we can process the whole block at once
    blockend = 0;
    for (cur = blockstart; blockend == 0; ++cur) {
      if (cur == encoded_end)
        return false;
      if (*cur == '-' && *(cur - 1) == '-' && cur+3+boundrystring.size()<encoded_end &&   //+3 includes the \r\n
          std::string(cur+1, boundrystring.size()) == boundrystring)
        blockend = cur - 3;    // found it (go back to the \r (as we're at the 2nd - in \r\n--
    }

    // block processing begin
    // process the header lines
    if (blockstart != blockend) {
      cur = anc = blockstart;
      ++cur;
      mid = 0;

      // process all the header lines
      field_name.clear();
      field_filename.clear();
      field_contenttype.clear();
      while (true) {
        // find the end of this line
        while ( !(*(cur-1) == '\r' && *cur == '\n') ) {
          ++cur;
          if (cur==blockend)
            return false; // come on, proper lines please
        }//while
        ++cur; //cur now points to AFTER the \r\n

        if (anc + 2 == cur)
          break;  // break out, this is an empty line and we now start the body

        // this will be the full line
        line.assign(anc, cur - 2 - anc);
        anc = cur;  // lets move the anchor to the next line

//OUTPUT << "LINE=(" << line << ")\n";
        std::string linename, lineval;
        if (!split_char(line, ':', linename, lineval))
          continue;

        linename = lowercase(linename);
        trim_left(lineval);
        trim_right(lineval);

        if (linename == "content-type")
          field_contenttype = lineval;
        else if (linename == "content-disposition") {
          std::vector<std::string> values;

          string_tokenize_word(lineval, values, "; ");

          if (values.empty() || values[0] != "form-data")
            return false;

          std::string left, right;
          for (int x=1; x<values.size(); ++x) {
            if (!split_char(values[x], '=', left, right))
              continue;

            if (left == "name")
              url_decode(right, field_name);
            else if (left == "filename")
              url_decode(right, field_filename);
          }//small for
        }//if content-disposition
      }//while true

      // header lines done, now process the body and insert the form data
      trim_quote(field_name);
      if (!field_name.empty()) {
        // insert the main data item/file
        m_form_data[field_name].assign(cur, blockend-cur);    // one less copy than insert()

        if (!field_filename.empty()) {
          trim_quote(field_filename);
          if (!field_filename.empty())
            m_form_data[member_field_name(field_name, "filename")] = field_filename;
        }

        if (!field_contenttype.empty())
          m_form_data[member_field_name(field_name, "content_type")] = field_contenttype;
      }//!field_name.empty
    }//big if
    // block processing end

/*for (form_t::iterator ii=m_form_data.begin(); ii!=m_form_data.end(); ++ii) {
  OUTPUT << "FORMFIELD=" << ii->first << "=";
  if (ii->second.size()>1024)
    OUTPUT << "BIG! " << ii->second.size() << " bytes\n";
  else
    print_binary(ii->second.c_str(), ii->second.size());
}*/
    
    // block processing complete, lets setup blockstart for the next one
    blockstart = blockend + 4 + boundrystring.size(); // 4 is \r\n--
    if (blockstart + 1<encoded_end && *blockstart == '-' && *(blockstart+1) == '-')
      return true;    // natural --X-- marker found
    blockstart += 2;  // skip over \r\n
    if (blockstart > encoded_end) // well, we're out of buffer, lets just give up and be ok
      return true;
  }

  assert(false);      // we should never get here
  // assume success
  return false;
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

void wexus::core::print_binary(const void *data, size_t datalen)
{
  if (!data || datalen == 0) {
    OUTPUT << "0[[]]\n";
    return;
  }

  OUTPUT << datalen << "\n[[";

  const char *ii = reinterpret_cast<const char *>(data);
  const char *endii = ii + datalen;
  for (; ii != endii; ++ii)
    //if (isprint(*ii) || *ii == '\n' || *ii == '\r')
    if (*ii == '\r')
      OUTPUT << "\\r";
    else if (*ii == '\n')
      OUTPUT << "\\n\n";
    else if (isprint(*ii))
      OUTPUT << *ii;
    else
      OUTPUT << "0x" << hexchars[(*ii & 0xF0) >> 4] << hexchars[*ii & 0x0F];

  OUTPUT << "]]\n";
}


