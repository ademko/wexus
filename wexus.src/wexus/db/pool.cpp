
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

#include <wexus/db/pool.h>

#include <scopira/tool/prop.h>
#include <scopira/tool/util.h>

#include <wexus/db/connection.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus;

db::connection_pool::connection_pool(void)
{
}

db::connection_pool::~connection_pool()
{
}

bool db::connection_pool::init_prop(scopira::tool::property *prop)
{
  const std::string* sz  = prop->get_string_value("db_poolsize");
  const std::string* dsn = prop->get_string_value("db_dsn");
  const std::string* n   = prop->get_string_value("db_username");
  const std::string* pw  = prop->get_string_value("db_password");

  int isz;
  if (sz && dsn && n && pw && string_to_int(*sz, isz) && isz>0)
    return init_pool(isz, *dsn, *n, *pw); // eveutanlly call init_pool

  return false;
}

bool db::connection_pool::init_pool(int conncount, const std::string &dsn, const std::string &username, const std::string &pw)
{
  locker_ptr<shared_t> p(m_connpool);
  connstack_t::iterator ii, endii;

  assert(conncount>0);
  assert(&dsn);
  assert(&username);
  assert(&pw);

  assert(p.get());

  p->next = 0;
  p->connstack.resize(conncount);

  endii = p->connstack.end();
  for (ii=p->connstack.begin(); ii!=endii; ++ii) {
    *ii = new db::connection(m_env);
    assert(&(*ii));
    if (!(*ii)->connect(dsn, username, pw)) {
      // it was a good try...
      assert(false && "[Failed to open ODBC connect]");
      p->connstack.resize(0);
      return false;
    }
  }

  return true;
}

void db::connection_pool::pop(scopira::tool::count_ptr< connection > &conn)
{
  locker_ptr<shared_t> p(m_connpool);
  // we're not leavin until we have a conn!
  while (true) {
    if (p->next < p->connstack.size()) {
      // got one!
      conn = p->connstack[p->next];
      p->next++;
      conn->set_autocommit(true);
      return;
    } else {
      // wait until someone gives one back
      m_connpool.pm_condition.wait(m_connpool.pm_mutex);
      p.reset();
    }
  }
}

void db::connection_pool::push(const scopira::tool::count_ptr< connection > &conn)
{
  locker_ptr<shared_t> p(m_connpool);

  assert(p->next >0);
  assert(p->next <= p->connstack.size());

  p->next--;

  p->connstack[p->next] = conn;

  // notify, incase someone was waiting
  m_connpool.pm_condition.notify_all();
}

