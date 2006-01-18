
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

#ifndef __INCLUDED_WEXUS_CORE_QUITFRONT_HPP__
#define __INCLUDED_WEXUS_CORE_QUITFRONT_HPP__

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
    /**
     * returns true, if the given filename-flag thingie
     * is in the "keep running state.
     *
     * keep running means the file exists and has a file size of exactly 0
     * bytes
     *
     * @author Aleksander Demko
     */ 
    bool test_filequit(const std::string &filename);

    class file_quitfront;

    //class signal_quitfront;     // a front that listens on a process signall
                                  // under UNIX, these are sent with the kill command
    //class service_quitfront;    // a front that knows how to stop to
                                  // win32 "stop service" requests
    //class key_quitfront;        // a front that stops when a certain key
                                  // combo is pressed (mainly in win32)
  }
}

/**
 * a file_quitmonitor that is front-compatible
 *
 * @author Aleksander Demko
 */ 
class wexus::core::file_quitfront : private scopira::tool::thread,
  public wexus::core::front_i
{
  public:
    /// default ctor
    file_quitfront(void);
    /// constructor
    file_quitfront(front_peer_i *peer, const std::string &filename);
    /// destructor
    virtual ~file_quitfront();

    /// load params from a prop tree
    virtual bool init_prop(front_peer_i *peer, scopira::tool::property *prop);

    /// thread

    /// signals stoppage
    virtual void notify_stop(void);
    /// the run core
    virtual void run(void);

  private:
    volatile bool m_alive;
    std::string m_filename;
};

#endif

