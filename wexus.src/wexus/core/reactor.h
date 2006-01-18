
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

#ifndef __INCLUDED_WEXUS_CORE_REACTOR_HPP__
#define __INCLUDED_WEXUS_CORE_REACTOR_HPP__

#include <string>

#include <scopira/tool/object.h>

namespace wexus
{
  namespace core
  {
    class reactor_i;
    // forward
    class app_event;
  }
}

/**
 * reactor interface. most user-apps will use the prebuild, supplied
 * reactors in core. in certain specialized cases they will also
 * make their own
 *
 * @author Aleksander Demko
 */
class wexus::core::reactor_i : public virtual scopira::tool::object
{
  public:
    /**
     * called when its time to handle an event
     *
     * this version goes through the reactor stack, looking for
     * a reactor to handle this event
     *
     * @return true if this reactor handled the event
     * @author Aleksander Demko
     */ 
    virtual bool handle_app_event(app_event &evt) = 0;
};

#endif

