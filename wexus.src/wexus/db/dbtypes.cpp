
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

#include <wexus/db/dbtypes.h>

#include <scopira/tool/time.h>
#include <scopira/tool/flow.h>
#include <scopira/tool/util.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus::db;

error::error(void)
{
  m_state[0] = 0;
  m_native = 0;
  m_msg[0] = 0;
}

bool error::set(handle &h, short recnum)
{
  SQLRETURN Ret;
  SQLSMALLINT len;

  Ret = SQLGetDiagRec(h.get_type(), h.m_handle, recnum, m_state, &m_native, m_msg, len_c, &len);
  m_state[5] = 0; // append a null to state

  return Ret == SQL_SUCCESS;
}

void error::print(oflow_i &out) const
{
  out << reinterpret_cast<const char*>(m_state) << '(' << static_cast<int>(m_native)
    << "): " << reinterpret_cast<const char*>(m_msg) << '\n';
}

void error::print_all(handle &h, oflow_i &out)
{
  short recnum = 1;
  error e;

  while (e.set(h, recnum)) {
    e.print(out);
    recnum++;
  }
}

dbstring::dbstring(const dbstring &s)
{
  init(s.m_max_size);
  set(s);
}

dbstring::dbstring(const char* s)
{
  // add 1 for the '/0' character
  init(strlen(s)+1);
  // set string
  set(s);
}

dbstring::dbstring(const std::string& s)
{
  // add 1 for the '/0' character
  init(s.size()+1);
  // set string
  set(s);
}

dbstring::dbstring(size_t len)
{
  assert(len > 2);
  init(len);
}

dbstring::~dbstring()
{
  delete []m_buf;
}

bool dbstring::load(scopira::tool::iobjflow_i& in)
{
  size_t readsize;

  if (!in.read_size_t(readsize))
    return false;

  if (readsize == 0) {
    m_cur_size = 0;
    m_buf[0] = 0;
    return true;
  }

  assert(readsize+1 <= m_max_size);
  assert(readsize>0);
  if (!in.read_void(m_buf, readsize))
    return false;
  m_cur_size = readsize;
  m_buf[m_cur_size] = 0;    // terminate it

  return true;
}

void dbstring::save(scopira::tool::oobjflow_i& out) const
{
  out.write_size_t(m_cur_size);
  if (m_cur_size>0)
    out.write_void(m_buf, m_cur_size);
}

void dbstring::init(int len)
{
  m_buf = new SQLCHAR[len];
  m_max_size = len;
  m_cur_size = 0;
  m_buf[0] = 0;
}

void dbstring::set(const dbstring &s)
{
  m_cur_size = s.m_cur_size;

  if (m_cur_size > m_max_size-1)
    m_cur_size = m_max_size-1;
  memcpy(m_buf, s.m_buf, m_cur_size);
  m_buf[m_cur_size] = 0;    // terminate it
}

void dbstring::set(const char *s)
{
  assert(s);
  m_cur_size = strlen(s);

  if (m_cur_size > m_max_size-1)
    m_cur_size = m_max_size-1;
  memcpy(m_buf, s, m_cur_size);
  m_buf[m_cur_size] = 0;    // terminate it
}

void dbstring::set(const std::string &s)
{
  m_cur_size = s.size();

  if (m_cur_size > m_max_size-1)
    m_cur_size = m_max_size-1;
  memcpy(m_buf, s.c_str(), m_cur_size);
  m_buf[m_cur_size] = 0;    // terminate it
}

void dbstring::set_int(int x)
{
  m_cur_size = int_to_string(x, reinterpret_cast<char*>(m_buf), m_max_size) - 1;
  assert(m_cur_size>=0);
}

void dbstring::set_double(double x)
{
  m_cur_size = double_to_string(x, reinterpret_cast<char*>(m_buf), m_max_size) - 1;
  assert(m_cur_size>=0);
}

void dbstring::clear(void)
{
  m_cur_size = 0;
  m_buf[0] = 0;
}

void dbstring::str(std::string &out) const
{
  out = reinterpret_cast<const char*>(m_buf);
}

std::string dbstring::str(void) const
{
  std::string ret;

  ret = reinterpret_cast<const char*>(m_buf);
  return ret;
}

dbdate::dbdate(void)
{
  clear();
}

dbdate::dbdate(SQLSMALLINT y, SQLUSMALLINT m, SQLUSMALLINT d)
{
  year = y;
  month = m;
  day = d;
}

dbdate::dbdate(const timestamp& t)
{
  year = t.get_year()+1900;
  month = t.get_month();
  day = t.get_day();
}

void dbdate::set(SQLSMALLINT y, SQLUSMALLINT m, SQLUSMALLINT d)
{
  year = y;
  month = m;
  day = d;
}

void dbdate::clear(void)
{
  year = 0;
  month = 0;
  day = 0;
}

timestamp dbdate::get_time(void) const
{
  return timestamp(get_year(), get_month(), get_day(), 0, 0, 0);
}

dbtimestamp::dbtimestamp(void)
{
  clear();
}

dbtimestamp::dbtimestamp(const timestamp& t)
{
  set_year(t.get_year() + 1900);
  set_month(t.get_month()+1);
  set_day(t.get_day());
  set_hour(t.get_hour());
  set_minute(t.get_minute());
  set_second(t.get_second());
  set_fraction(0);
}

timestamp dbtimestamp::get_time(void) const
{
  return timestamp(get_year(), get_month(), get_day(), get_hour(), get_minute(), get_second());
}

dbdate today(void)
{
  timestamp t = timestamp::current_time();
  return dbdate(t.get_year()+1900, t.get_month()+1, t.get_day());
}

void dbtimestamp::clear(void)
{
  set_year(0);
  set_month(0);
  set_day(0);
  set_hour(0);
  set_minute(0);
  set_second(0);
  set_fraction(0);
}

dbtimestamp now(void)
{
  timestamp t = timestamp::current_time();

  dbtimestamp ret;
  ret.set_year(t.get_year() + 1900);
  ret.set_month(t.get_month()+1);
  ret.set_day(t.get_day());
  ret.set_hour(t.get_hour());
  ret.set_minute(t.get_minute());
  ret.set_second(t.get_second());
  ret.set_fraction(0);

  return ret;
}

scopira::tool::oflow_i& operator<<(scopira::tool::oflow_i& o, dbdate& v)
{
  return o << v.get_year() << '-' << v.get_month() << '-' << v.get_day();
}

