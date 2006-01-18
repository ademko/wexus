
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

#include <wexus/core/datacache.h>

#include <wexus/core/handlerreactor.h>

#include <wexus/db/dbrec.h>
#include <wexus/db/statement.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus;

core::dbrec_cache_base::dbrec_cache_base(void)
{
  m_dirty = true;
}

core::dbrec_cache_base::~dbrec_cache_base()
{
  // nothing for now
}

void core::dbrec_cache_base::flush(void)
{
  write_locker W(m_qlock);

  m_query = 0;
}

void core::dbrec_cache_base::set_dirty(void)
{
  write_locker W(m_qlock);

  m_dirty = true;
}

void core::dbrec_cache_base::flush_dirty(void)
{
  {
    read_locker R(m_qlock);

    if (!m_dirty)
      return; // no need to flush, not dirty
  }
  {
    write_locker W(m_qlock);

    // dirty, flush and reset dirty flag
    m_dirty = false;
    m_query = 0;
  }
}

count_ptr<rec::rec_i> core::dbrec_cache_base::get(db::connection &cn)
{
  {
    read_locker R(m_qlock);

    if (!m_query.is_null())
      return m_query.get();
  }
  // null, reload and return
  {
    write_locker W(m_qlock);

    refresh(cn, m_query);
    assert(!m_query.is_null());
    return m_query.get();
  }
}

count_ptr<rec::rec_i> core::dbrec_cache_base::get(core::handler_event &e)
{
  {
    read_locker R(m_qlock);

    if (!m_query.is_null())
      return m_query.get();
  }
  // null, reload and return
  {
    write_locker W(m_qlock);

    refresh(e.db(), m_query);
    assert(!m_query.is_null());
    return m_query.get();
  }
}

core::static_dbrec_cache::static_dbrec_cache(const std::string &query)
  : m_query_string(query)
{
}

void core::static_dbrec_cache::refresh(db::connection &cn, count_ptr<db::mem_db_rec> &qout)
{
  // use a statement per get call... this is cleaner since i only need a connection
  // from the outside world, but slightly slower. but since this will be called
  // infrequently (as its a cache refresher), it shouldnt matter
  db::statement st(cn);

  if (st.execute(m_query_string))
    qout = new db::mem_db_rec(st);
}

