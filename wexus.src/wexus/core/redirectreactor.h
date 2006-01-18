
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

#ifndef __INCLUDED__WEXUS_CORE_REDIRECTREACTOR_HPP__
#define __INCLUDED__WEXUS_CORE_REDIRECTREACTOR_HPP__

#include <string>
#include <map>

#include <wexus/core/reactor.h>

namespace wexus
{
  namespace core
  {
    class redirect_reactor;
  }
}

/**
 * redirects users, for a fixed set of req's
 *
 * @author Aleksander Demko
 */
class wexus::core::redirect_reactor : public virtual wexus::core::reactor_i
{
  public:
    /// default ctor
    redirect_reactor(void);
    /// same as default, + one add_url call
    redirect_reactor(const std::string &req, const std::string &url);
    /// dtor
    ~redirect_reactor();

    /**
     * called when its time to handle an event
     *  
     * this version goes through the reactor stack, looking for
     * a reactor to handle this event
     *
     * @return true if this reactor handled the event
     * @author Marc Jeanson
     */ 
    virtual bool handle_app_event(app_event &evt);
    
    /**
     * adds a new url redirect
     *
     * @param req the request string
     * @param url the url to redirect them to
     * @author Aleksander Demko
     */ 
    void add_url(const std::string &req, const std::string &url);

  private:
    typedef std::map<std::string, std::string> urlmap_t;

    urlmap_t m_urlmap;
};

#endif

