
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

#ifndef __INCLUDED__WEXUS_CORE_DEFAULTREACTOR_HPP__
#define __INCLUDED__WEXUS_CORE_DEFAULTREACTOR_HPP__

#include <wexus/core/reactor.h>

namespace wexus
{
  namespace core
  {
    class default_reactor;
  }
}

/**
 * a reactor that reditects all requests to another reactor.
 * good for default actions
 *
 * @author Aleksander Demko
 */
class wexus::core::default_reactor : public virtual wexus::core::reactor_i
{
  public:
    /// default ctor
    default_reactor(const std::string &defreq, reactor_i *defreactor);
    /// dtor
    virtual ~default_reactor();

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

  private:
    std::string m_defreq;     /// the default req string
    reactor_i *m_defreactor;   /// the reactor to call
};

#endif

