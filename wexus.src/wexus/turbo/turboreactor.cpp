
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

#include <wexus/turbo/turboreactor.h>

#include <scopira/tool/thread.h>
#include <wexus/turbo/register.h>
#include <wexus/turbo/session.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus::core;
using namespace wexus::turbo;

static scopira::tool::thread::tlskey_t g_tlskey;

turbo_event & wexus::turbo::get_tls_event(void)
{
  void *v;

  v = thread::get_tls(g_tlskey);

  assert(v);

  return *reinterpret_cast<turbo_event*>(v);
}

//
//
// turbo_event_data
//
//

turbo_event_data::turbo_event_data(core::session_manager *sesmgr, db::connection_pool *p)
  : m_sesmgr(sesmgr), m_conn_pool(p)
{
}

turbo_event_data::~turbo_event_data()
{
  if (m_cur_conn.get())
    m_conn_pool->push(m_cur_conn);
}

wexus::db::connection * turbo_event_data::get_connection(void)
{
  if (m_cur_conn.get())
    return m_cur_conn.get();    // return existing
  if (!m_conn_pool)
    return 0;  // no connection pool to draw from
  m_conn_pool->pop(m_cur_conn);  // check out one
  return m_cur_conn.get();
}

void turbo_event_data::add_error(const std::string &msg)
{
  m_errors.insert(stringmap_t::value_type("", msg));
}

void turbo_event_data::add_error(const std::string &name, const std::string &msg)
{
  m_errors.insert(stringmap_t::value_type(name, msg));
}

string_iterator turbo_event_data::get_errors(void) const
{
  return string_iterator(m_errors.begin(), m_errors.end());
}

string_iterator turbo_event_data::get_errors(const std::string &key) const
{
  return string_iterator(m_errors.lower_bound(key), m_errors.upper_bound(key));
}

void turbo_event_data::clear_errors(void)
{
  m_errors.clear();
}

void turbo_event_data::add_note(const std::string &msg)
{
  m_notes.insert(stringmap_t::value_type("", msg));
}

void turbo_event_data::add_note(const std::string &name, const std::string &msg)
{
  m_notes.insert(stringmap_t::value_type(name, msg));
}

string_iterator turbo_event_data::get_notes(void) const
{
  return string_iterator(m_notes.begin(), m_notes.end());
}

string_iterator turbo_event_data::get_notes(const std::string &key) const
{
  return string_iterator(m_notes.lower_bound(key), m_notes.upper_bound(key));
}

void turbo_event_data::clear_notes(void)
{
  m_notes.clear();
}

//
//
// turbo_event
//
//

turbo_event::html_encoder_flow::html_encoder_flow(bool doref, scopira::tool::oflow_i *out)
  : m_out(doref, out)
{
}

size_t turbo_event::html_encoder_flow::write(const byte_t* _buf, size_t _size)
{
  const byte_t *b, *e, *endii;
  bool ok;

  if (_size == 0)
    return 0;

  endii = _buf + _size;

  while (true) {
    // mark b and e for the good block
    b = _buf;
    e = _buf;
    ok = true;
    while (ok) {
      if (e == endii)
        break;
      switch (*e) {
        case '"':
        case '&':
        case '<':
        case '>':
          ok = false; break;
      default:
          ++e;
      }//switch
    }//while ok

    // ok, send out b->e verbatim
    if (b != e)
      m_out->write(b, e-b);

    // no, deal with what stopped us
    if (e == endii)
      break;

    // special char
    switch (*e) {
      case '"': *m_out << "&quot;"; break;
      case '&': *m_out << "&amp;"; break;
      case '<': *m_out << "&lt;"; break;
      case '>': *m_out << "&gt;"; break;
    }

    //finally, increment
    _buf = e + 1;
  } //while true

  return _size;
}

void turbo_event::set_session(session_i *ses)
{
  assert(turbo.get_session_manager());

  if (ses)
    turbo.get_session_manager()->set_session(front, ses);
  else
    if (session)
      turbo.get_session_manager()->clear_session(front, session->get_sessionid());

  session = ses;
}

scopira::tool::oflow_i & turbo_event::html_output(void)
{
  if (html_encoder.is_null())
    html_encoder = new html_encoder_flow(false, &output());
  return *html_encoder;
}

void turbo_event::add_next_note(const std::string &msg)
{
  add_next_note("", msg);
}

void turbo_event::add_next_note(const std::string &name, const std::string &msg)
{
  user_session *us = dynamic_cast<user_session*>(session);

  assert(us && "[called add_next_note without a valid, current wexus::turbo::user_session]\n");

  us->notes.insert(user_session::notemap_t::value_type(name, msg));
}

const std::string & turbo_event::get_form_field(const std::string &fieldname) const
{
  if (turbo.has_formcache_field(fieldname))
    return turbo.formcache_field(fieldname);

  return app_event::get_form_field(fieldname);
}

bool turbo_event::has_form_field(const std::string& name) const
{
  return turbo.has_formcache_field(name) || front.has_form_field(name);
}

void turbo_event::set_form_field(const std::string &fieldname, const std::string &val)
{
  turbo.formcache_field(fieldname) = val;
}

void turbo_event::set_form_field_default(const std::string &fieldname, const std::string &val)
{
  if (!front.has_form_field(fieldname))
    turbo.formcache_field(fieldname) = val;
}

//
//
// turbo_reactor
//
//

turbo_reactor::turbo_reactor(core::session_manager *sesmgr, db::connection_pool *dbpoolmgr)
  : m_sesmgr(sesmgr), m_dbpoolmgr(dbpoolmgr)
{
  assert(sesmgr);

  m_reg = registry::instance();
  assert(m_reg);

  // this should probably be move dto a more singleton like class (registry, or wexus_loop)
  thread::open_tls(g_tlskey);
}

turbo_reactor::~turbo_reactor()
{
  thread::close_tls(g_tlskey);
}

bool turbo_reactor::handle_app_event(core::app_event &evt)
{
  registry::tfunc_t f = m_reg->get_type(evt.req);

  if (f) {
    turbo_event_data dat(m_sesmgr, m_dbpoolmgr);
    turbo_event evt2(evt.req, evt.front, evt.app, dat, 0);
    count_ptr<session_i> ses;
    user_session *us = 0;

    if (m_sesmgr) {
      assert(!evt2.session);
      
      ses = m_sesmgr->get_session(evt2.front);
      if (ses.get())
        evt2.session = ses.get();

      us = dynamic_cast<user_session*>(ses.get());
    }

    thread::set_tls(g_tlskey, &evt2);

    // need to do this inside the TLS setup
    if (us)
      us->flush_notes();
    /// finally, call the Wexus Turbo Function (WTF :)
    f(evt2);

    thread::set_tls(g_tlskey, 0);

    return true;
  }

  return false;
}

