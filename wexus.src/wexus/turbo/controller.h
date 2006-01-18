
/*
 *  Copyright (c) 2006    Aleksander B. Demko
 *
 *  All rights reserved.
 *
 *  This material is confidential and proprietary information of
 *  Aleksander B. Demko ("Confidential Information").
 *  This Confidential Information may only be used and reproduced
 *  in accordance with the terms of the license agreement.
 *
 */

#ifndef __INCLUDED_WEXUS_TURBO_CONTROLLER_H__
#define __INCLUDED_WEXUS_TURBO_CONTROLLER_H__

#include <string>
#include <typeinfo>

#include <scopira/tool/object.h>
#include <wexus/db/dbtypes.h>
#include <wexus/core/http.h>
#include <wexus/turbo/widgets.h>
#include <wexus/turbo/formatters.h>

namespace wexus
{
  namespace turbo
  {
    class turbo_controller;
  }
}

/**
 * A base controller object class in the turbo based controller system.
 * (the C in MVC, I guess).
 *
 * @author Aleksander Demko
 */
class wexus::turbo::turbo_controller
{
  public:
    /// ctor
    turbo_controller(void);
    /// dtor
    virtual ~turbo_controller();

  protected:
    /**
     * This class is instantied, by default, in most wx handlers.
     * It does a generic header in the ctor, and footer in the dtor.
     *
     * You can make your own standard_header class in your coordinator
     * decents, or simply change your wx code.
     *
     * @author Aleksander Demko
     */ 
    class standard_header
    {
      public:
        standard_header(const std::string &pagetitle = "Wexus App");
        ~standard_header();
    };
};

#endif

