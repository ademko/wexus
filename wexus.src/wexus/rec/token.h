
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

#ifndef __INCLUDED_WEXUS_REC_TOKEN_HPP__
#define __INCLUDED_WEXUS_REC_TOKEN_HPP__

#include <list>

#include <scopira/tool/object.h>

namespace wexus
{
  namespace rec
  {
    class token_i;
    // forward
    class rec_i;
  }
}

/**
 * base token produced by the system
 *
 * @author Aleksander Demko
 */ 
class wexus::rec::token_i : public virtual scopira::tool::object
{
  public:
    typedef std::list< rec_i* > recstack_t;

  public:
    /**
     * executes this token against a SaltRec stack while writing the
     * output to the given stream
     *
     * @param _recstack the stack (first==top) of SaltRec objects
     * @param _out the output stream
     * @author Aleksander Demko
     */
    virtual void run_token(recstack_t & recstack, scopira::tool::oflow_i &output) = 0;
};

#endif

