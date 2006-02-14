
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

#ifndef __INCLUDED_WEXUS_CORE_HTTPFRONT_H__
#define __INCLUDED_WEXUS_CORE_HTTPFRONT_H__

#include <vector>
#include <list>

#include <scopira/tool/bufferflow.h>
#include <scopira/tool/netflow.h>

#include <wexus/core/httpevent.h>
#include <wexus/core/http.h>

namespace wexus
{
  namespace core
  {
    class front_peer_i;

    class http_front;
    class http_worker;
  }
}

/**
 * Front class that repsonds to HTTP requests, using an internal pool of http_worker.
 * Rewritten to be smaller/tighter and with some accept-flow control to avoid
 * flooding the worker pool.
 *
 * @author Aleksander Demko
 */
class wexus::core::http_front : public scopira::tool::thread,
  public virtual wexus::core::front_i
{
  friend class http_worker;
  public:
    typedef std::list<scopira::tool::count_ptr<scopira::tool::netflow> > clientq_t;

    /// default ctor
    http_front(void);
    /// initing ctor
    http_front(wexus::core::front_peer_i* peer, int thread_count, int port=80, size_t maxpost=0);

    /// load params from a prop tree
    virtual bool init_prop(wexus::core::front_peer_i *peer, scopira::tool::property *prop);

    // thread
    virtual void run(void);

    // front_i
    virtual void start(void);
    virtual void notify_stop(void);
    virtual void wait_stop(void);

  private:
    typedef std::vector<scopira::tool::count_ptr<http_worker> > workers_t;

    struct client_area {
      /// queue of unbound clients from the server
      clientq_t clients;
      /// we all alive?
      bool alive;
    };

    /// share are with the workers
    scopira::tool::event_area<client_area> m_clientarea;
    /// http server port
    int m_port;
    /// max post size, in bytes
    size_t m_maxpost;
    /// the actual threads
    workers_t m_workers;
    /// a looper
    scopira::tool::net_loop m_netloop;
};


/**
 * A worker thread that is owned by the http_front.
 * This class does the actual parsing and processing of a client socket after
 * it's accepted by the server (http_process) thread.
 *
 * @author Aleksander Demko
 */
class wexus::core::http_worker : public scopira::tool::thread
{
  public:
    /// ctor
    http_worker(http_front *master, int myid);

    /// main run method
    virtual void run(void);

  protected:
    /// returns last status code
    int process_event(scopira::tool::netflow &clientflow);
    // helpers for process event
    void parse_request(http_event& evt);
    void send_to_engine(http_event& evt);

    bool decode_request_line(const std::string& request_line, http_event& evt);

  private:
    http_front *m_master;
    int m_myid;   // my thread number, sometimes used for debugging
};

#endif


