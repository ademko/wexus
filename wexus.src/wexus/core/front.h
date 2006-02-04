
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
#include <scopira/tool/netflow.h>
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
    class front_i;
    class front_peer_i;
  }
}

/**
 * The core event interface that binds the engine (and its applications)
 * to the remote client.
 *
 * @author Aleksander Demko
 */ 
class wexus::core::front_event_i
{
  public:
    virtual ~front_event_i() { }
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
     * @author Aleksander Demko
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
     * @author Aleksander Demko
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
     * @author Aleksander Demko
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

    /**
     * returns the ip of the remote client. this returns false if thats
     * not available or not relevant.
     * @author Aleksander Demko
     */ 
    virtual bool get_client_addr(scopira::tool::netaddr &out) const = 0;
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
    virtual ~front_peer_i() { }
  public:
    /// the front should call this on event request
    virtual void handle_front_event(front_event_i& evt) = 0;

    /// this signals the peer to begin total shutdown
    virtual void handle_shutdown(void) = 0;
};

#endif

