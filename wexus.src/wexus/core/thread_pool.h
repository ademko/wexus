
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

#ifndef __INCLUDED_WEXUS_TOOL_THREADPOOL_H__
#define __INCLUDED_WEXUS_TOOL_THREADPOOL_H__

#include <scopira/tool/thread.h>
#include <vector>

namespace wexus
{
  namespace core
  {
    template <class T> class thread_data;
    template <class T> class thread_pool;
    template <class T> class pool_thread;
  }
}

/**
 * A class that combines data and an associated shutdown
 * flag to be used by threads in a thread pool.
 * @param data type to be used with a thread pool and it's threads.
 *
 * @author Andrew Kaspick
 */
template <class T>
class wexus::core::thread_data
{
  public:
    thread_data(void) : m_shutdown(false) {}

    T     m_data;
    bool  m_shutdown;
};

/**
 * The thread type that is to be used with the thread_pool class.
 * @param data type that the thread is working with
 *
 * @author Andrew Kaspick
 */
template <class T>
class wexus::core::pool_thread : public scopira::tool::thread
{
  public:
    pool_thread(scopira::tool::event_area<thread_data<T> >& data)
      : thread(0), m_shared_data(data) {}

  protected:
    scopira::tool::event_area<thread_data<T> > &  m_shared_data;
};

/**
 * thread pool class 
 * @param data type that the thread pool is working with
 *
 * @author Andrew Kaspick
 */
template <class T>
class wexus::core::thread_pool : public virtual scopira::tool::job_i
{
  public:
    thread_pool(int num_threads) : m_num_threads(num_threads) {}

    virtual void start(void)
    {
      scopira::tool::locker_ptr<thread_data<T> > lk_ptr(m_shared_data);
      lk_ptr->m_shutdown = false;

      create_threads();

      for (typename thread_pool_t::iterator it = m_threads.begin(); it != m_threads.end(); it++)
        (*it)->start();
    }

    virtual void notify_stop(void)
    {
      {
        scopira::tool::locker_ptr<thread_data<T> > lk_ptr(m_shared_data);
        lk_ptr->m_shutdown = true;
      }

      // notify any threads waiting on the shared data
      m_shared_data.pm_condition.notify_all();
    }

    virtual void wait_stop(void)
    {
      for (typename thread_pool_t::iterator it = m_threads.begin(); it != m_threads.end(); it++)
      {
        assert((*it)->is_alive_object());
        (*it)->wait_stop();
      }
    }

    virtual bool is_running(void) const
    {
      scopira::tool::locker_ptr<thread_data<T> > lk_ptr(m_shared_data);

      return !lk_ptr->m_shutdown;;
    }
    
    scopira::tool::event_area<thread_data<T> > & get_data(void) { return m_shared_data; }

    virtual pool_thread<T>* create_thread(void) = 0;

  private:
    void create_threads(void)
    {
      for (int i=0; i<m_num_threads; i++)
      {
        pool_thread<T>* t = create_thread();
        m_threads.push_back(t);
      }
    }

    int m_num_threads;

    typedef std::vector<scopira::tool::count_ptr<pool_thread<T> > > thread_pool_t;
    thread_pool_t m_threads;

    scopira::tool::event_area<thread_data<T> >  m_shared_data;
};

#endif

