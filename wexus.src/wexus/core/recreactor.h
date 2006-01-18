
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

#ifndef __INCLUDED__WEXUS_CORE_RECREACTOR_HPP__
#define __INCLUDED__WEXUS_CORE_RECREACTOR_HPP__


#include <string>
#include <vector>
#include <map>

#include <wexus/core/reactor.h>
#include <wexus/core/recmanager.h>

#include <wexus/rec/rec.h>
#include <wexus/rec/token.h>

namespace wexus
{
  namespace core
  {
    class rec_reactor;
  }
}

/**
 * tempaltes a static dir with root rec
 *
 * @author Aleksander Demko
 */
class wexus::core::rec_reactor : public virtual wexus::core::reactor_i
{
  public:
    /**
     * construct a new rec-filing reactor
     *
     * @param mimetype the mime type to send back
     * @param mgr the rec_manager to use
     * @author Aleksander Demko
     */ 
    rec_reactor(wexus::rec::rec_i *rootrec, wexus::core::rec_manager *mgr,
      const std::string &mimetype = "text/html");
    /// dtor
    virtual ~rec_reactor();

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
    
  protected:
    wexus::rec::rec_i *m_rootrec;       /// root rec
    wexus::core::rec_manager *m_mgr;    /// the manager
    std::string m_mimetype;               /// mime type to send back
};

#endif

