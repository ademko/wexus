
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

#ifndef __INCLUDED__WEXUS_CORE_HTTP_H__
#define __INCLUDED__WEXUS_CORE_HTTP_H__

#include <string>
#include <map>
#include <vector>
#include <list>

#include <scopira/tool/time.h>
#include <scopira/tool/flow.h>

namespace wexus
{
  namespace core
  {
    class http_status;

    class http_headers;
    class http_cookies;
    class http_server_cookies;
    class http_form;

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

    /// debug routine
    void print_binary(const void *data, size_t datalen);
  }
}

/**
 * A class to handle HTTP status codes
 *
 * @author Aleksander Demko
 */
class wexus::core::http_status
{
  public:
    http_status(void) : m_status_set(false), m_code(0) {}

    void set_code(int code);
    int  get_code(void) const { return m_code; }
    const std::string& get_message(void) const { return m_message; }

  private:
    bool m_status_set;
    int m_code;
    std::string m_message;
};

/**
 * A collection of http headers, from the cilent.
 * 
 * @author Aleksander Demko
 */
class wexus::core::http_headers
{
  public:
    typedef std::map<std::string, std::string> headers_t;

    headers_t pm_headers;

    /// parses header information in http request
    bool parse_headers(std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last);
    /// parses header information for the provided header
    bool parse_header(const std::string& header, const std::string& data);
};

/**
 * A mapping of cookie name and values
 *
 * @author Aleksander Demko
 */ 
class wexus::core::http_cookies
{
  public:
    /// ctors
    http_cookies(void);

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

/**
 * A collection of cookies that will be sent from the server
 * to the client.
 *
 * @author Aleksander Demko
 */
class wexus::core::http_server_cookies
{
  public:
    // a public class for now

    /// one cookie
    struct cookie_t
    {
      std::string name;
      std::string value;
      std::string expires;
      std::string domain;
      std::string path;
    };

    /// the cookies
    typedef std::list<cookie_t> cookies_t;

    /// finally, the instance variable
    cookies_t pm_cookies;

    void send_server_cookies(scopira::tool::oflow_i &client) const;
};

/**
 * The mapping of form fields.
 *
 * @author Aleksander Demko
 */ 
class wexus::core::http_form
{
  public:
    /// ctors
    http_form(void);

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
    typedef std::map<std::string, std::string> form_t;
    form_t m_form_data;
};

#endif

