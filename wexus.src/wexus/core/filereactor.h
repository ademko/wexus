
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

#ifndef __INCLUDED_WEXUS_CORE_FILEREACTOR_HPP__
#define __INCLUDED_WEXUS_CORE_FILEREACTOR_HPP__

#include <wexus/core/reactor.h>

//#include <scopira/tool/flow.h>

namespace wexus
{
  namespace core
  {
    class file_reactor;
    // forward
    class file_manager;
  }
}

/**
 * reactor interface. most user-apps will use the prebuild, supplied
 * reactors in core. in certain specialized cases they will also
 * make their own
 *
 * @author Marc Jeanson
 */
class wexus::core::file_reactor : public virtual wexus::core::reactor_i
{
  public:
    /// ctor
    file_reactor(wexus::core::file_manager *mgr);
    /// dtor
    virtual ~file_reactor();

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
    wexus::core::file_manager *m_mgr;
};

#endif

