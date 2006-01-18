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

#ifndef __INCLUDED__WEXUS_TOOL_HTTP_H__
#define __INCLUDED__WEXUS_TOOL_HTTP_H__

#include <string>
#include <map>
#include <vector>

#include <scopira/tool/time.h>

namespace wexus
{
  namespace core
  {
    class cookies;
    class form_data;

    /**
     * performs url decoding (on any string)
     * decodes %xx values
     *
     * @param encoded encoded url
     * @param decoded decoded url
     * @param form form data is being processed
     * @author Andrew Kaspick
     */ 
    void url_decode(const std::string& encoded, std::string& decoded, bool form=false);

    /**
     * performs url encoding (on any string)
     * encodes non-alpha numeric values to a %xx values
     *
     * @param str string to be encoded
     * @param decoded url encoded string
     * @param form form data is being processed
     * @author Andrew Kaspick
     */ 
    void url_encode(const std::string& str, std::string& encoded, bool form=false);

    // friendly version of above
    std::string url_encode(const std::string& str, bool form=false);

    /// quote encode
    void html_quote_encode(const std::string& str, std::string& out);
    // friendly version of above
    const std::string html_quote_encode(const std::string &str);
    /// quote decode
    void html_quote_decode(const std::string& str, std::string& out);

    /// tag encode - might be different in the future?
    inline void html_tag_encode(const std::string& str, std::string& out) { out = str; }

    /// makes a timestamp from an http_format string
    scopira::tool::timestamp httptime_to_timestamp(const std::string &http_format_time);
    /// converts a time stamp to http format
    std::string timestamp_to_httptime(scopira::tool::timestamp t);
  }
}

class wexus::core::cookies
{
  public:
    /// ctors
    cookies(void);

    /// checks for a cookie of the given name
    bool has_cookie(const std::string& name) const;

    /**
     * get the cookies of the given name
     *
     * @param name name of the cookie to retreive
     * @param the values found for the given name
     * @author Andrew Kaspick
     */
    void get_cookies(const std::string& name, std::vector<std::string>& values) const;

    /// parse cookie information from header string or body string
    bool decode_and_parse(const std::string& raw_cookie_str);

  private:
    /// multimap of cookie names and their associated values
    typedef std::multimap<std::string, std::string> cookies_t;
    cookies_t m_cookies;
};

class wexus::core::form_data
{
  public:
    /// ctors
    form_data(void);

    /// checks if the form field exists
    bool has_field(const std::string& name) const;

    /// get the value of the provided form field
    const std::string& get_field(const std::string& name) const;

    /**
     * decodes a url with form data
     *
     * @param encoded encoded url
     * @return were the values successfully decoded into form name/value pairs
     * @author Andrew Kaspick
     */
    bool decode_and_parse(const std::string& encoded);

  private:
    /// map of form name/value pairs
    typedef std::map<std::string, std::string> form_data_t;
    form_data_t m_form_data;
};

#endif

