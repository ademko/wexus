
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

#include <wexus/core/handlerreactor.h>

#include <scopira/tool/fileflow.h>
#include <scopira/tool/util.h>

#include <wexus/core/app.h>
#include <wexus/core/front.h>
#include <wexus/core/sessionmanager.h>

#include <wexus/db/pool.h>
#include <wexus/db/connection.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus;

core::handler_event_data::handler_event_data(db::connection_pool *p)
  : m_conn_pool(p)
{
  //FIXME? remove this?
  m_rootrec.set_rec("errors", new rec::mem_rec);
}

core::handler_event_data::~handler_event_data()
{
  if (m_cur_conn.get()) {
    m_rootrec.clear();  // i must clear this before puttin back the connection,
      // because i need to unref any db_recs that might have built up
    m_conn_pool->push(m_cur_conn);
  }
}

db::connection * core::handler_event_data::get_connection(void)
{
  if (m_cur_conn.get())
    return m_cur_conn.get();    // return existing
  if (!m_conn_pool)
    return 0;  // no connection pool to draw from
  m_conn_pool->pop(m_cur_conn);  // check out one
  return m_cur_conn.get();
}

void core::handler_event_data::add_error(const std::string &msg)
{
  rec::mem_rec *r;

  if (!m_rootrec.has_rec("errors"))
    m_rootrec.set_rec("errors", r = new rec::mem_rec);
  else
    r = dynamic_cast<rec::mem_rec*>(m_rootrec.get_rec("errors"));

  assert(r);
  r->set_rec(int_to_string(r->get_rec_size()), new rec::msg_rec(msg));
}

void core::handler_event_data::add_error(const std::string &name, const std::string &msg)
{
  rec::mem_rec *r;

  if (!m_rootrec.has_rec("errors"))
    m_rootrec.set_rec("errors", r = new rec::mem_rec);
  else
    r = dynamic_cast<rec::mem_rec*>(m_rootrec.get_rec("errors"));

  assert(r);
  r->set_rec(name, new rec::msg_rec(msg));
}

bool core::handler_event_data::has_error(void)
{
  //FIXME? fix this?
  return m_rootrec.has_rec("errors") && m_rootrec.get_rec("errors")->get_rec_size()>0;
}

void core::handler_event::set_session(session_manager *sesman, session_i *ses)
{
  assert(sesman);

  if (ses)
    sesman->set_session(front, ses);
  else
    if (session)
      sesman->clear_session(front, session->get_sessionid());

  session = ses;
}

core::handler_reactor::handler_reactor(rec_manager *mgr, session_manager *sesmgr,
  db::connection_pool *dbpoolmgr)
  : m_mgr(mgr), m_sesmgr(sesmgr), m_dbpoolmgr(dbpoolmgr)
{
  assert(mgr);
}

bool core::handler_reactor::handle_app_event(app_event &evt)
{
  handlermap_t::iterator ii = m_handlermap.find(evt.req);

  if (ii != m_handlermap.end()) {
    handler_event_data handerdat(m_dbpoolmgr);
    handler_event evt2(evt.req, evt.front, evt.app, handerdat, 0);
    handler_i* h = ((*ii).second).get();

    if (m_sesmgr) {
      assert(!evt2.session);
      session_i *ses;
      
      ses = m_sesmgr->get_session(evt2.front);
      if (ses)
        evt2.session = ses;
    }

    h->handle_app_event(evt2);
    post_template(evt2);

    return true;
  }
  return false;
}

void core::handler_reactor::add_handler(const std::string &req, handler_i* handler)
{
  m_handlermap[req] = handler;
}

void core::handler_reactor::post_template(handler_event &evt)
{
  const std::string& temfname = evt.handler.get_template();

  if (!temfname.empty())
    m_mgr->merge_template(temfname, evt.handler.get_root(), evt.front.get_output());
}

