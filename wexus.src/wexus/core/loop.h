
/*
 *  Copyright (c) 2005    Aleksander B. Demko
 *
 *  All rights reserved.
 *
 *  This material is confidential and proprietary information of
 *  Aleksander B. Demko ("Confidential Information").
 *  This Confidential Information may only be used and reproduced
 *  in accordance with the terms of the license agreement.
 *
 */

#ifndef __INCLUDED_WEXUS_CORE_LOOP_H__
#define __INCLUDED_WEXUS_CORE_LOOP_H__

#include <scopira/core/loop.h>

namespace wexus
{
  namespace core
  {
    class wexus_loop;
  }
}

/**
 * Main loop for wexus apps
 *
 * @author Aleksander Demko
 */
class wexus::core::wexus_loop : public scopira::core::basic_loop
{
  public:
    /// ctor
    wexus_loop(int &argc, char **&argv);
    /// dtor
    ~wexus_loop();
};

#endif

