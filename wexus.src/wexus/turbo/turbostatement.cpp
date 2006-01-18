
/*
 *  Copyright (c) 2005    Aleksander B. Demko
 *
 *  All rights reserved.
 *
 *  This material is confidential and proprietary information of
 *  Aleksander B. Demko ("Confidential Information").
 *  This Confidential Information may only be used and reproduced
 *  in accordance with the terms of the license agreement.
 *
 */

#include <wexus/turbo/turbostatement.h>

#include <wexus/turbo/exception.h>
#include <wexus/turbo/event.h>

#include <scopira/tool/output.h>

//BBtargets libwexus.so

using namespace wexus::turbo;
using namespace wexus::db;

turbo_statement::turbo_statement(void)
  : statement(EVENT.db()), m_fresh(true)
{
  m_query.reserve(256);
}


turbo_statement::turbo_statement(connection &conn)
  : statement(conn), m_fresh(true)
{
  m_query.reserve(256);
}

turbo_statement & turbo_statement::operator << (dbstring &v)
{
  if (!m_fresh)
    internal_reset();

  if (m_query.empty())
    bind_col_string(&v);
  else {
    m_query += " ? ";
    bind_param_string(&v);
  }
  
  return *this;
}

turbo_statement & turbo_statement::operator << (dbint &v)
{
  if (!m_fresh)
    internal_reset();

  if (m_query.empty())
    bind_col_int(&v);
  else {
    m_query += " ? ";
    bind_param_int(&v);
  }
  
  return *this;
}

turbo_statement & turbo_statement::operator << (const char *c)
{
  assert(c);

  if (!m_fresh)
    internal_reset();

  if (c[0] == 0) // empty string
    return *this;

  m_query += c;
  
  return *this;
}

turbo_statement & turbo_statement::operator << (char c)
{
  bool b;
  switch (c) {
    case 1: //preparef
      m_fresh = false;
      b = prepare(m_query);
      if (!b) {
        OUTPUT << "preparef() - prepare() error=" << error(*this) << " query=" << m_query << '\n';
        throw db_error("turbo_statement preparef-variant exception");
      }
      break;
    case 3: //prepare_executef
      m_fresh = false;
      b = prepare(m_query);
      if (!b) {
        OUTPUT << "prepare_executef() - prepare() error=" << error(*this) << " query=" << m_query << '\n';
        throw db_error("turbo_statement preparef-variant exception");
      }
      b = execute();
      if (!b) {
        OUTPUT << "prepare_executef() - execute() error=" << error(*this) << " query=" << m_query << '\n';
        throw db_error("turbo_statement preparef-variant exception");
      }
      break;
    case 4: // prepare_execute_fetchf
      m_fresh = false;
      b = prepare(m_query);
      if (!b) {
        OUTPUT << "prepare_execute_fetchf() - prepare() error=" << error(*this) << " query=" << m_query << '\n';
        throw db_error("turbo_statement preparef-variant exception");
      }
      b = execute();
      if (!b) {
        OUTPUT << "prepare_execute_fetchf() - execute() error=" << error(*this) << " query=" << m_query << '\n';
        throw db_error("turbo_statement preparef-variant exception");
      }
      b = fetch_next();
      if (!b) {
        OUTPUT << "prepare_execute_fetchf() - fetch_next() error=" << error(*this) << " query=" << m_query << '\n';
        throw db_error("turbo_statement preparef-variant exception");
      }
      break;
    default: //otherwise just add the one char to the query
      m_query += c;
      break;
  }
  return *this;
}

const std::string & turbo_statement::operator [](const std::string &colname) const
{
  short colnum;
  if (!find_colnum(colname, colnum))
    t_assert(false, "The column doesn't exist: " + colname);

  return statement::operator[](colname);
}

void turbo_statement::internal_reset(void)
{
  close();
  m_query.clear();
  m_fresh = true;
}

turbo_xtion::turbo_xtion(bool defaultcommit)
  : db(EVENT.db())
{
  m_userset = false;
  m_commit = defaultcommit;

  db.set_autocommit(false);
}

turbo_xtion::~turbo_xtion()
{
  if (!m_userset && std::uncaught_exception())
    m_commit = false;

  if (m_commit)
    db.commit();
  else
    db.rollback();

  db.set_autocommit(true);
}

