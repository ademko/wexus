
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

#ifndef __INCLUDED__WEXUS_CORE_LISTREACTOR_HPP__
#define __INCLUDED__WEXUS_CORE_LISTREACTOR_HPP__

#include <wexus/core/reactor.h>

#include <list>

namespace wexus
{
  namespace core
  {
    class list_reactor;
    class ifmatch_reactor;
  }
}

/**
 * propagates the event to a list of reactors
 *
 * @author Aleksander Demko
 */
class wexus::core::list_reactor : public virtual wexus::core::reactor_i
{
  public:
    /// default ctor
    list_reactor(void);
    /// dtor
    virtual ~list_reactor();

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

    /**
     * adds a reactor to the end of my list. it will be
     * ref counted
     *
     * @author Aleksander Demko
     */ 
    void add_reactor(reactor_i *r);

  private:
    typedef std::list< scopira::tool::count_ptr< reactor_i > > rlist_t;

    rlist_t m_rlist;    /// the list of reactors

};

/**
 * executes its list of reactors if the request matches the given string
 *
 * @author Aleksander Demko
 */
class wexus::core::ifmatch_reactor : public wexus::core::list_reactor
{
  public:
    /// where to match
    enum { whole_c, subset_c, begin_c, end_c };

    /**
     * ctor, with the match string to look for
     *
     * @author Aleksander Demko
     */ 
    ifmatch_reactor(const std::string& match, int where, bool not_op = false);
    /**
     * ctor
     * matches against an encoded string that starts with following format:
     * [!] {<>=} string
     * Where ! is an OPTIONAL not operator
     * <>= are the operators. choose ONE. < is prefix match, > is suffix,
     * and = is whole (thats right, there is no subset)
     * string is your string, can be 0-length
     *
     * @author Andrew Kaspick
     */
    ifmatch_reactor(const std::string& encodedmatch);
    /// dtor
    virtual ~ifmatch_reactor();

    /**
     * called when its time to handle an event
     *  
     * this version goes through the reactor stack, looking for
     * a reactor to handle this event
     *
     * @return true if this reactor handled the event
     * @author Andrew Kaspick
     */ 
    virtual bool handle_app_event(app_event& evt);

  private:
    std::string m_matchstring;      /// the string to match against
    int         m_where;            /// where to match
    bool        m_not_op;           /// not the operation
};

#endif

