
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

#ifndef __INCLUDED_WEXUS_ENGINE_STRING_HPP__
#define __INCLUDED_WEXUS_ENGINE_STRING_HPP__

#include <string>

namespace wexus
{
  namespace core
  {
    /// managed by the engine
    extern const std::string * empty_string;
  }
}

#define EMPTYSTRING (*wexus::core::empty_string)

#endif
