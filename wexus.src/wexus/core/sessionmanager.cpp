
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

#include <wexus/core/sessionmanager.h>

#include <vector>

#include <scopira/tool/util.h>
#include <scopira/tool/prop.h>
#include <scopira/tool/archiveflow.h>
#include <scopira/core/register.h>

#include <wexus/core/front.h>
#include <wexus/core/handlerreactor.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus;

//
//
// session_i
//
//

core::session_i::session_i(void)
  : m_sessionid(0)
{
  touch_stamp();
}

bool core::session_i::load(scopira::tool::iobjflow_i& in)
{
  touch_stamp();

  return in.read_int(m_sessionid);
}

void core::session_i::save(scopira::tool::oobjflow_i& out) const
{
  out.write_int(m_sessionid);
}

int core::session_i::get_sessionid(void) const
{
  read_locker lk(m_stamprw);

  return m_sessionid;
}

void core::session_i::prune(bool properlogoff)
{
}

void core::session_i::touch_stamp(void)
{
  write_locker lk(m_stamprw);

  m_stamp = timestamp::current_time();
}

timestamp core::session_i::get_stamp(void) const
{
  read_locker lk(m_stamprw);

  return m_stamp;
}

//
//
// session_manager
//
//

core::session_manager::session_manager(void)
{
}

core::session_manager::session_manager(const std::string &cookiename, const std::string &cookiepath, const std::string &cookiedomain)
  : m_cookiename(cookiename), m_cookiepath(cookiepath), m_cookiedomain(cookiedomain)
{
}

core::session_manager::~session_manager()
{
  prune_all();
}

bool core::session_manager::init_prop(scopira::tool::property *prop)
{
  const std::string *n, *p, *d;

  n = prop->get_string_value("cookie_name");
  p = prop->get_string_value("cookie_path");
  d = prop->get_string_value("cookie_domain");

  if (n && p && d) {
    m_cookiename = *n;
    m_cookiepath = *p;
    m_cookiedomain = *d;
    return true;
  }

  return false;
}

void core::session_manager::set_session(session_i *ses)
{
  assert(ses);
  assert(ses->get_sessionid() != 0);
  write_locker lk(m_seslock);
  m_sessions[ses->get_sessionid()].set(ses);
}

void core::session_manager::clear_session(int id)
{
  write_locker lk(m_seslock);

  // clearing current session
  sesmap_t::iterator ii;

  assert(id != 0);
  ii = m_sessions.find(id);
  if (ii != m_sessions.end()) {
    (*ii).second->prune(true);
    m_sessions.erase(ii);   // remove the session
  }
}

core::session_i * core::session_manager::get_session(int id)
{
  if (id == 0)
    return 0;

  count_ptr<core::session_i> loaded_session;;

  {
    read_locker lk(m_seslock);
    sesmap_t::iterator ii;

    ii = m_sessions.find(id);
    if (ii == m_sessions.end()) {
      // not found, lets try the file store, IF we're using it
      if (!m_filestore.empty()) {
        archiveiflow inf;

        inf.open(m_filestore + int_to_string(id), "WEXUSSESSION100");

        if (!inf.failed())
          inf.read_object_type(loaded_session);
      }

      // drop to the 2nd half of this function
    } else {
      core::session_i *ret;
      ret = (*ii).second.get();
      ret->touch_stamp();
      return ret;
    }
  }

  // ok, write teh loaded_session and return it
  if (loaded_session.is_null())
    return 0;   // not found
  loaded_session->touch_stamp();
  {
    write_locker lk(m_seslock);

    m_sessions[id] = loaded_session;

    return loaded_session.get();
  }
}

core::session_i * core::session_manager::get_session(front_event_i &he)
{
  if (!he.has_client_cookie(m_cookiename))
    return 0;

  int id;
  std::vector<std::string> cookies;

  he.get_client_cookies(m_cookiename, cookies);
  // check if we an id and grab the session for it
  if (!cookies.empty() && string_to_int(cookies[0], id))
    return get_session(id);

  return 0;
}

void core::session_manager::set_session(front_event_i &he, session_i *ses)
{
  write_locker lk(m_seslock);

  assert(ses);
  assert(ses->get_sessionid() == 0);
  ses->set_sessionid(next_id());

  m_sessions[ses->get_sessionid()].set(ses);

  he.set_server_cookie(m_cookiename, int_to_string(ses->get_sessionid()),
    m_cookiepath, -1, m_cookiedomain);

  if (!m_filestore.empty()) {
    // store it to file
    archiveoflow outf;

    outf.open(m_filestore + int_to_string(ses->get_sessionid()), "WEXUSSESSION100");

    outf.write_object(ses);
  }
}

void core::session_manager::clear_session(front_event_i &he, int id)
{
  write_locker lk(m_seslock);

  // clearing current session
  sesmap_t::iterator ii;

  assert(id != 0);
  ii = m_sessions.find(id);
  if (ii != m_sessions.end()) {
    (*ii).second->prune(true);
    m_sessions.erase(ii);   // remove the session
  }

  he.set_server_cookie(m_cookiename, int_to_string(0),
    m_cookiepath, -1, m_cookiedomain);

  if (!m_filestore.empty()) {
    // store null to file
    archiveoflow outf;

    outf.open(m_filestore + int_to_string(id), "WEXUSSESSION100");

    outf.write_object(0);
  }
}

void core::session_manager::prune(void)
{
  write_locker lk(m_seslock);

  sesmap_t::iterator ii, nukeii;
  timestamp cur;

  cur = timestamp::current_time();

  ii = m_sessions.begin();
  while (ii != m_sessions.end())
    if ((cur - (*ii).second->get_stamp()).get_minutes() >= 30) {    // replace thsi with a 30 min check
      // nuke it, its old
      (*ii).second->prune(false);
      nukeii = ii;
      ++ii;
      m_sessions.erase(nukeii);
    } else
      ++ii; // just skip
}

void core::session_manager::prune_all(void)
{
  write_locker lk(m_seslock);

  sesmap_t::iterator ii, endii;

  // call their prune handlers before we nuke em
  endii = m_sessions.end();
  for (ii = m_sessions.begin(); ii != endii; ++ii)
    (*ii).second->prune(false);
}

void core::session_manager::set_file_store(const std::string &basedir)
{
  write_locker lk(m_seslock);

  m_filestore = basedir;

  if (!m_filestore.empty())
    m_filestore += "/wexus_session_";
}

int core::session_manager::next_id(void) const
{
  int i;

  // to predicable, use /dev/urandom or something?
  // or UUIDs!
  do {
    i = ::rand();
  } while (const_cast<sesmap_t*>(&m_sessions)->count(i) > 0);

  return i;
}

