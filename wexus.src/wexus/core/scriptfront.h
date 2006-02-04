
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

#ifndef __INCLUDED_WEXUS_CORE_SCRIPTFRONT_HPP__
#define __INCLUDED_WEXUS_CORE_SCRIPTFRONT_HPP__

#include <scopira/tool/thread.h>
#include <wexus/core/front.h>

namespace scopira
{
  namespace tool
  {
    class oflow_i;
  }
}

namespace wexus
{
  namespace core
  {
    class scriptfront;
  }
}

/**
 * TODO This class used to exist, but didn't keep up with API changes.
 * When needed, this class will be made to:
 *   - process a file/callable from programmers test main line
 *   - it will make virtual requests to the engine
 *   - support cookies (ie. for session)
 *   - perhaps verify output, or atleast status return codes
 *
 * Again, this will be reinspected as the need arises.
 *
 * @author Aleksander Demko
 */
class wexus::core::scriptfront : private scopira::tool::thread,
  public virtual wexus::core::front_i
{
  public:
    /// ctor
    scriptfront(void);
    /// constructor
    scriptfront(const std::string &filename, front_peer_i *peer);
    /// destructor
    virtual ~scriptfront();

    /// load params from a prop tree
    virtual bool init_prop(front_peer_i *peer, scopira::tool::property *prop);

    /// thread

    /// the run core
    virtual void run(void);

    // front_i

  protected:
    class script_front_event : public wexus::core::front_event_i
    {
      public:
    };

  private:
    std::string m_filename; /// filename to work with, if any
};

#endif

