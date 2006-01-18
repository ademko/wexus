
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

#ifndef __INCLUDED_WEXUS_HTTPFRONT_H__
#define __INCLUDED_WEXUS_HTTPFRONT_H__

#include <vector>
#include <list>

#include <scopira/tool/bufferflow.h>
#include <scopira/tool/netflow.h>

#include <wexus/core/httpevent.h>
#include <wexus/core/http.h>
#include <wexus/core/thread_pool.h>

namespace wexus
{
  namespace core
  {
    class front_peer_i;

    class response_status;
    class http_thread;
    class http_front;
  }
}

/**
 * A class to handle HTTP status codes
 *
 * @author Andrew Kaspick
 */
class wexus::core::response_status
{
  public:
    response_status() : m_status_set(false), m_code(0) {}

    void set_code(int code);
    int  get_code(void) const { return m_code; }
    const std::string& get_message(void) const { return m_message; }

  private:
    bool        m_status_set;
    int         m_code;
    std::string m_message;
};

/**
 * Front class that repsonds to HTTP requests.
 *
 * @author Andrew Kaspick
 */
class wexus::core::http_front : public virtual scopira::tool::thread,
  public virtual wexus::core::front_i
{
  friend class http_thread;
  public:
    typedef std::list<scopira::tool::count_ptr<scopira::tool::netflow> > clientq_t;

    /// default ctor
    http_front(void);
    /// initing ctor
    http_front(wexus::core::front_peer_i* peer, int thread_count, int port=80);

    /// load params from a prop tree
    virtual bool init_prop(wexus::core::front_peer_i *peer, scopira::tool::property *prop);

    /// thread

    /// the run core
    virtual void run();

    /// front_i

    /// starts the job
    virtual void start(void);

    /// tells the job to try to stop
    virtual void notify_stop(void);

    /// this blocks until the job stops
    virtual void wait_stop(void);

  private:
    class http_thread_pool : public wexus::core::thread_pool<clientq_t>
    {
      public:
        http_thread_pool(wexus::core::front_peer_i* peer, int num_threads);
        virtual wexus::core::pool_thread<clientq_t>* create_thread(void);

        scopira::tool::shared_area<int> m_data;

      private:
        wexus::core::front_peer_i* m_peer;
    };

    /// http server port
    int m_port;
    /// http servers thread pool
    scopira::tool::count_ptr<http_thread_pool> m_thread_pool;
    /// a looper
    scopira::tool::net_loop m_netloop;
};


/**
 * The thread type to be used in the http_front classes thread pool.
 *
 * @author Andrew Kaspick
 */
class wexus::core::http_thread :
  public wexus::core::pool_thread<wexus::core::http_front::clientq_t>
{
  public:
    http_thread(
      wexus::core::front_peer_i* peer,
      scopira::tool::event_area<wexus::core::thread_data<http_front::clientq_t> >& data,
      wexus::core::http_front::http_thread_pool& pool
    );

    virtual ~http_thread();

    virtual void run();

    static const std::string& http_version(void) { return m_http_version; }

  protected:
    /// map of header name value pairs
    typedef std::map<std::string, std::string> headers_t;

    /// data received from the client request
    struct request_t
    {
      std::string method;
      std::string uri;
      std::string version;

      // headers: includes unparsed cookie data
      headers_t headers;

      // parsed cookies received from client
      wexus::core::cookies client_cookies;

      // parsed form data
      wexus::core::form_data form_data;

      // unformatted (binary) body of request
      scopira::tool::bufferflow body;

      // http response status
      response_status status;
    };

    /// event to be used with http requests
    class event : public virtual wexus::core::http_event_imp
    {
      public:
        event(scopira::tool::bufferflow& buffer, scopira::tool::netflow& raw_buffer, request_t& request);
        virtual ~event() {}

        virtual void set_request(const std::string& request) { i_set_request(request); }

        virtual scopira::tool::oflow_i& get_output(void) { return m_buffer; }

        virtual scopira::tool::oflow_i& get_raw_output(void);

        virtual void send_file(const std::string& name, size_t speed = 0);

        /// sends response line and header data
        bool send_headers(void);
        /// sends body of the response
        void send_body(void);

      protected:
        /// send an individual cookie
        virtual void send_server_cookie(const std::string& cookie);

        bool check_headers(void);
        void check_status(void);

      private:
        scopira::tool::bufferflow& m_buffer;
        scopira::tool::netflow&    m_raw_buffer;

        request_t& m_request;
        bool m_sent_headers;    // have the headers been sent?
        bool m_sending_headers; // are the headers being sent?
    };

    /// get the request from the client
    void get_request(request_t& request);
    /// parses request data
    void parse_request(request_t& request);
    /// handle a request from the client
    void send_reply(request_t& request);

    /// decode request line
    bool decode_request_line(const std::string& request_line, request_t& request);

    /// parses header information in http request
    bool parse_headers(std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last, headers_t& headers);
    /// parses header information for the provided header
    bool parse_header(const std::string& header, const std::string& data, headers_t& headers);

  private:
    wexus::core::front_peer_i* m_peer;

    scopira::tool::count_ptr<scopira::tool::netflow>  m_client;

    /// supported HTTP version
    static const std::string      m_http_version;

    // ** temporary values for debugging
    http_front::http_thread_pool& m_pool;
};

#endif


