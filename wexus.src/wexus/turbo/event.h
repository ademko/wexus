
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

#ifndef __INCLUDED_WEXUS_TURBO_EVENT_H__
#define __INCLUDED_WEXUS_TURBO_EVENT_H__

/*
 * This (kind of special) header declares the EVENT symbol
 * and includes a bunch of other things that are handy.
 *
 */

/**
 * This file is automatically included by all .cpp files that were created
 * from .wx files by wxmake
 *
 * @author Aleksander Demko
 */ 

#include <wexus/turbo/turboreactor.h>

#define EVENT (wexus::turbo::get_tls_event())

#include <wexus/turbo/register.h>
#include <wexus/core/checkers.h>
#include <wexus/db/connection.h>
#include <wexus/db/statement.h>
#include <wexus/turbo/widgets.h>
#include <wexus/turbo/formatters.h>
#include <wexus/turbo/validators.h>
#include <wexus/turbo/session.h>
#include <wexus/turbo/turbostatement.h>

namespace wexus
{
  namespace turbo
  {
    /// adds a error msg to the root
    //inline void add_error(const std::string &msg) { EVENT.add_error(msg); }
    /// adds a error msg to the root, with a name
    //inline void add_error(const std::string &name, const std::string &msg) { EVENT.add_error(name, msg); }
    /// has any string_iterator?
    //inline bool has_error(void) { return EVENT.has_error(); }
  }
}


#endif

