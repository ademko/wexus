
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

#ifndef __INCLUDED_WEXUS_CORE_TICKMANAGER_HPP__
#define __INCLUDED_WEXUS_CORE_TICKMANAGER_HPP__

#include <vector>

#include <scopira/tool/thread.h>

namespace wexus
{
  namespace core
  {
    class tick_event;

    // these might need tweaking to look like the handler.h ones... or maybe use a common base
    class ticker_i;
    class function_ticker_0;
    class function_ticker_1;
    template <class T> class method_ticker_0;
    template <class T> class method_ticker_1;

    class tick_manager;
  }
}

/**
 * a tick event object sent to tickers
 *
 * @author Aleksander Demko
 */
class wexus::core::tick_event
{
  public:
    int dummy;    // put stuff here later
    // add just-in-time db handle here
};

/**
 * something that accepts ticks
 *
 * @author Aleksander Demko
 */
class wexus::core::ticker_i : public virtual scopira::tool::object
{
  public:
    /// handles a tick
    virtual void handle_tick(tick_event &evt) = 0;
};

/**
 * function that takes 0 params
 *
 * @author Aleksander Demko
 */
class wexus::core::function_ticker_0 : public wexus::core::ticker_i
{
  public:
    typedef void(*ticker)(void);

    /// ctor
    function_ticker_0(ticker t);
    /// handles a tick
    virtual void handle_tick(tick_event &evt);
  private:
    ticker m_t;
};

/**
 * function that takes 1 param, the event object
 *
 * @author Aleksander Demko
 */ 
class wexus::core::function_ticker_1 : public wexus::core::ticker_i
{
  public:
    typedef void(*ticker)(tick_event&);

    /// ctor
    function_ticker_1(ticker t);
    /// handles a tick
    virtual void handle_tick(tick_event &evt);
  private:
    ticker m_t;
};

/**
 * method that takes 0 params
 *
 * @author Aleksander Demko
 */
template <class T>
  class wexus::core::method_ticker_0 : public wexus::core::ticker_i
{
  public:
    typedef void (T::*ticker)(void);

    /// ctor
    method_ticker_0(T *obj, ticker t)
      : m_obj(obj), m_t(t) { }
    /// handles a tick
    virtual void handle_tick(tick_event &evt)
      { (m_obj->*m_t)(); }
  private:
    T * m_obj;
    ticker m_t;
};

/**
 * method that takes 1 param, the tick_event
 *
 * @author Aleksander Demko
 */
template <class T>
  class wexus::core::method_ticker_1 : public wexus::core::ticker_i
{
  public:
    typedef void (T::*ticker)(tick_event&);

    /// ctor
    method_ticker_1(T *obj, ticker t)
      : m_obj(obj), m_t(t) { }
    /// handles a tick
    virtual void handle_tick(tick_event &evt)
      { (m_obj->*m_t)(evt); }
  private:
    T * m_obj;
    ticker m_t;
};

/**
 * manager of ticks (constantly repeating events, like UNIX-cron)
 *
 * @author Aleksander Demko
 */
class wexus::core::tick_manager : public scopira::tool::thread
{
  public:
    /// ctor
    tick_manager(void);
    /// dtor
    ~tick_manager(void);

    /// tells the tick manager to stop
    virtual void notify_stop(void);
    /// the thread run core
    virtual void run(void);

    /**
     * adds a ticker (will be ref counted) that will
     * be called every freq minutes
     *
     * @author Aleksander Demko
     */
    void add_ticker(int freq, ticker_i *t);

  private:
    typedef std::pair< int, scopira::tool::count_ptr<ticker_i> > tickpair_t;
    typedef std::vector< tickpair_t > tickvec_t;

    tickvec_t m_ticks;

    volatile bool m_alive;
};

#endif

