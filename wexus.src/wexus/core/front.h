
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

#ifndef __INCLUDED_WEXUS_CORE_FRONT_HPP_H__
#define __INCLUDED_WEXUS_CORE_FRONT_HPP_H__

#include <string>
#include <vector>
#include <map>

#include <scopira/tool/time.h>
#include <scopira/tool/thread.h>
#include <scopira/tool/object.h>
#include <wexus/core/http.h>

namespace scopira
{
  namespace tool
  {
    class property;
  }
}

namespace wexus
{
  namespace core
  {
    class front_event_i;
    class front_event_imp;
    class front_i;
    class front_peer_i;
  }
}

//
// these classes define the bridge between the fronts
// and the engine. front's should not include any other files
// from the engine namespace except this one
//

/**
 * front's pass front_event's to their peer (engine)
 *
 * @author Andrew Kaspick
 */
class wexus::core::front_event_i
{
  public:
    /// sets the content type of the event
    virtual void set_content_type(const std::string& type) = 0;

    /// Sets the content size of the event output.
    /// Not required, but recommended if using
    /// get_raw_output.
    virtual void set_content_size(int size) = 0;

    /// gets the request string (the requested uri)
    virtual const std::string& get_request(void) const = 0;

    /// gets the output flow
    virtual scopira::tool::oflow_i& get_output(void) = 0;

    /// gets the raw output flow
    virtual scopira::tool::oflow_i& get_raw_output(void) = 0;

    /**
     * Sends a file
     *
     * This method is preferred for sending file over get_output or get_raw_output
     * The content type must be set before calling this method.
     *
     * @param name the name of the file to send
     * @param speed the speed to throttle the upload at (in KB/sec). 0 for no throttle.
     * @author Andrew Kaspick
     */
    virtual void send_file(const std::string& name, size_t speed = 0) = 0;

    /**
     * creates a cookie to be sent to the client from the server
     *
     * @param name the name of the cookie
     * @param value the value of the cookie
     * @param path the path to set the cookie at
     * @param expires the expiry date of the cookie (default expires when user session ends)
     * @param the domain that the cookie is sent from
     * @author Andrew Kaspick
     */
    virtual void set_server_cookie(const std::string& name, const std::string& value,
      const std::string& path, scopira::tool::timestamp expires = -1, const std::string& domain = "") = 0;

    /// checks for a cookie sent from the client of the given name
    virtual bool has_client_cookie(const std::string& name) const = 0;

    /**
     * get the cookies of the given name that were sent to the server from the client.
     *
     * @param name name of the cookie to retreive
     * @param the values found for the given name
     * @author Andrew Kaspick
     */
    virtual void get_client_cookies(const std::string& name, std::vector<std::string>& values) const = 0;

    /// checks if the form field exists
    virtual bool has_form_field(const std::string& name) const = 0;

    /// get the value of the provided form field
    /// the field MUST exist
    virtual const std::string& get_form_field(const std::string& name) const = 0;

    /// sets a url to be directed to
    virtual void set_redirect_url(const std::string& url) = 0;

    /// sets the modified time of a resource
    virtual void set_modified_time(const scopira::tool::timestamp& time) = 0;

  protected:
    /// sets the request for the event
    virtual void set_request(const std::string& request) = 0;
};

/**
 * A default implementation of front_event_i
 *
 * @author Andrew Kaspick
 */
class wexus::core::front_event_imp : public wexus::core::front_event_i
{
  public:
    /**
     * constructor
     * 
     * @param content type for the event (default is text/html)
     * @author Andrew Kaspick
     */
    front_event_imp(
      const core::form_data* formdata = 0,
      const core::cookies* client_cookies = 0,
      const std::string& content_type = "text/html");
    /// destructor
    virtual ~front_event_imp() { }

    /// sets the content type of the event (stored as lower case)
    virtual void set_content_type(const std::string& type);

    /// gets the content type of the event
    virtual const std::string& get_content_type(void) const { return m_content_type; }

    /// Sets the content size of the event output.
    /// Not required, but recommended if using
    /// get_raw_output.
    virtual void set_content_size(int size) { m_content_size = size; }

    virtual int get_content_size(void) const { return m_content_size; }

    /// gets the request string (the requested uri)
    virtual const std::string& get_request(void) const { return m_request; }

    /// gets the raw output flow
    virtual scopira::tool::oflow_i& get_raw_output(void) { return get_output(); }

    /**
     * Sends a file
     *
     * This method is preferred for sending file over get_output or get_raw_output
     * The content type must be set before calling this method.
     *
     * @param name the name of the file to send
     * @param speed the speed to throttle the upload at (in KB/sec). 0 for no throttle.
     * @author Andrew Kaspick
     */
    virtual void send_file(const std::string& name, size_t speed = 0);

    /// creates a cookie to be sent to the client from the server
    virtual void set_server_cookie(const std::string& name, const std::string& value,
      const std::string& path, scopira::tool::timestamp expires = -1, const std::string& domain = "");

    /// checks for a cookie sent from the client of the given name
    virtual bool has_client_cookie(const std::string& name) const;

    /**
     * get the cookies of the given name that were sent to the server from the client.
     *
     * @param name name of the cookie to retreive
     * @param the values found for the given name
     * @author Andrew Kaspick
     */
    virtual void get_client_cookies(const std::string& name, std::vector<std::string>& values) const;

    /// checks if the form field exists
    virtual bool has_form_field(const std::string& name) const;

    /// get the value of the provided form field
    virtual const std::string& get_form_field(const std::string& name) const;

    /// sets a url to be directed to
    virtual void set_redirect_url(const std::string& url) { m_redirect_url = url; }

    /// gets the redirected url
    virtual const std::string& get_redirect_url(void) const { return m_redirect_url; }

    /// sets the modified time of a resource
    virtual void set_modified_time(const scopira::tool::timestamp& mod_time) { m_modified_time = mod_time; }

    /// gets the modified time of a resource
    virtual const scopira::tool::timestamp& get_modified_time(void) const { return m_modified_time; }

  protected:
    /// cookie data
    struct cookie_data_t
    {
      std::string value;
      std::string expires;
      std::string domain;
    };
    // key = cookie name
    typedef std::map<std::string, cookie_data_t> cookiemap_t;
    /// cookies sent from the server (key = cookie path)
    typedef std::multimap<std::string, cookiemap_t> server_cookies_t;

    /// internal method to be used by derived classes to set the request
    void i_set_request(const std::string& request) { m_request = request; }

    /// get the cookie information
    const server_cookies_t& get_server_cookies() const { return m_server_cookies; }

#ifdef DEBUG_THROTTLE
    void display_speed(scopira::tool::timestamp start_time, size_t sample_size, int sample_rate);
#endif

  private:
    std::string            m_content_type;   // content type of event
    int                    m_content_size;   // content size of event output
    std::string            m_request;        // request string
    const core::form_data* m_formdata;       // form data (name/value pairs)
    const core::cookies*   m_client_cookies; // cookies received from the client
    server_cookies_t       m_server_cookies; // cookies set on the server
    std::string            m_redirect_url;   // redirected url
    scopira::tool::timestamp             m_modified_time;  // modified time of resource
};

/**
 * this is how an engine views a front. all front's must implement this
 * interface
 *
 * @author Aleksander Demko
 */
class wexus::core::front_i : public virtual scopira::tool::job_i
{
  public:
    /// default ctor
    front_i(void) : m_peer(0) { }
    /// constructor
    front_i(front_peer_i* peer) : m_peer(peer) { }

    /// load params from a prop tree
    virtual bool init_prop(front_peer_i *peer, scopira::tool::property *prop) = 0;

  protected:
    /// returns the peer
    front_peer_i* get_peer(void) const { return m_peer; }
    /// sets the peer, meant to be called from init_prop impl
    void set_peer(front_peer_i *peer) {
      assert(!m_peer);
      m_peer = peer;
    }

  private:
    /// peer, if any
    front_peer_i* m_peer;
};

/**
 * what a front sends msgs to
 *
 * @author Aleksander Demko
 */ 
class wexus::core::front_peer_i
{
  public:
    /// the front should call this on event request
    virtual void handle_front_event(front_event_i& evt) = 0;

    /// this signals the peer to begin total shutdown
    virtual void handle_shutdown(void) = 0;
};

#endif

