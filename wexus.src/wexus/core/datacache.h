
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

#ifndef __INCLUDED_WEXUS_CORE_DATACACHE_HPP__
#define __INCLUDED_WEXUS_CORE_DATACACHE_HPP__

#include <scopira/tool/thread.h>
#include <wexus/rec/rec.h>

// stuff to aid in data caching

namespace wexus
{
  namespace core
  {
    class handler_event;    //fwd
    class dbrec_cache_base;
    class static_dbrec_cache;
  }

  namespace db
  {
    // forward
    class connection;
    class mem_db_rec;
  }
}

/**
 * a cached mem_db_rec. you need to make your own decendants of this class
 * and overide the refresh method
 *
 * @author Aleksander Demko
 */
class wexus::core::dbrec_cache_base
{
  public:
    /// dtor
    virtual ~dbrec_cache_base();

    /// flush the current item
    void flush(void);

    /// set the current item to be dirty, that is, dont flush it right
    void set_dirty(void);

    /// flush ONLY if dirty
    void flush_dirty(void);

    /// gets the current item, refreshes the cache if need be
    scopira::tool::count_ptr<rec::rec_i> get(db::connection &cn);

    /// gets the current item, refreshes the cache if need be
    scopira::tool::count_ptr<rec::rec_i> get(core::handler_event &e);

  protected:
    /// ctor
    dbrec_cache_base(void);

    /// decendant classes must implement this.
    /// when called, they should fill the given query rec
    virtual void refresh(db::connection &cn, scopira::tool::count_ptr<db::mem_db_rec> &qout) = 0;

  private:
    scopira::tool::count_ptr<db::mem_db_rec> m_query;   /// the cached query
    bool m_dirty;
    scopira::tool::rwlock m_qlock;                /// the lock for above
};

/**
 * a static (0 paramter) query cache
 *
 * @author Aleksander Demko
 */
class wexus::core::static_dbrec_cache : public wexus::core::dbrec_cache_base
{
  public:
    /// ctor
    static_dbrec_cache(const std::string &query);

  protected:
    /// when called, they should fill the given query rec
    virtual void refresh(db::connection &cn, scopira::tool::count_ptr<db::mem_db_rec> &qout);

  private:
    std::string m_query_string;
};

#endif

