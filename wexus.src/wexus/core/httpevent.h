
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

#ifndef __INCLUDED_WEXUS_CORE_HTTPEVENT_H__
#define __INCLUDED_WEXUS_CORE_HTTPEVENT_H__

#include <scopira/tool/bufferflow.h>
#include <wexus/core/front.h>
#include <wexus/core/http.h>

namespace wexus
{
  namespace core
  {
    class http_event;
  }
}

/**
 * The http front_event_i implementation and event object.
 * This is sent back to the engine to be handled by the various applications.
 *
 * @author Aleksander Demko
 */ 
class wexus::core::http_event : public wexus::core::front_event_i
{
  public:
    /**
     * Constructor. Initial stuff via direct member access (yes, it's ok because
     * this is a tightly coupled class with the http front anyways)
     * 
     * @author Aleksander Demko
     */
    http_event(scopira::tool::netflow &_rawbuf);

    // front_event_i stuff

    virtual void set_content_type(const std::string& type);
    virtual void set_content_size(int size) { pm_content_size = size; }
    virtual const std::string& get_request(void) const { return pm_uri; }
    virtual scopira::tool::oflow_i& get_output(void) { return pm_buffer; }
    virtual scopira::tool::oflow_i& get_raw_output(void);
    virtual void send_file(const std::string& name, size_t speed = 0);
    virtual void set_server_cookie(const std::string& name, const std::string& value,
      const std::string& path, scopira::tool::timestamp expires = -1, const std::string& domain = "");
    virtual bool has_client_cookie(const std::string& name) const;
    virtual void get_client_cookies(const std::string& name, std::vector<std::string>& values) const;
    virtual bool has_form_field(const std::string& name) const;
    virtual const std::string& get_form_field(const std::string& name) const;
    virtual void set_redirect_url(const std::string& url) { pm_redirect_url = url; }
    virtual void set_modified_time(const scopira::tool::timestamp& mod_time) { pm_modified_time = mod_time; }
    virtual bool get_client_addr(scopira::tool::netaddr &out) const { out = pm_client_addr; return true; }

    // http_front specific stuff

    /// creates an error page
    void create_error_page(int status, const std::string& message);

    /// sends response line and header data
    bool send_headers(void);
    /// sends body of the response
    void send_body(void);

  protected:
    bool check_headers(void);

    void check_status(void);

  public:   // yes, all public... just to keep things uncluttered
    std::string            pm_method;    /// GET, POST, etc
    std::string            pm_uri;       /// full URI
    std::string            pm_version;   /// HTTP/1.1 etc

    scopira::tool::bufferflow  pm_body;  /// unformatted (binary) body of request

    std::string            pm_content_type;   /// content type of event
    int                    pm_content_size;   /// content size of event output

    http_headers           pm_headers;        /// raw HTTP headers
    core::http_form        pm_formdata;       /// form data (name/value pairs)
    core::http_cookies     pm_client_cookies; /// cookies received from the client

    http_server_cookies    pm_server_cookies; /// cookies set locally, and to be sent to the client
    std::string            pm_redirect_url;   /// redirected url
    scopira::tool::timestamp   pm_modified_time;  /// modified time of resource
    scopira::tool::netaddr     pm_client_addr;    /// IP address of the client, initted to netflow's get_addr

    scopira::tool::bufferflow  pm_buffer;
    scopira::tool::netflow&    pm_raw_buffer;

    std::string            pm_boundry;       /// if not null, this is the border string in a multipart/form-data upload

    // http response status
    http_status            pm_status;          /// return http status code
    bool                   pm_sent_headers;    /// have the headers been sent?
    bool                   pm_sending_headers; /// are the headers being sent?
};

#endif

