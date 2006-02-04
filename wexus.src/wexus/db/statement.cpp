
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

#include <wexus/db/statement.h>

#include <wexus/db/connection.h>

#include <scopira/tool/util.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus::db;

statement::statement(connection &conn)
  : m_param_count(bind_start_count), m_col_count(bind_start_count), m_validfetch(false)
{
  SQLRETURN Ret;

  Ret = SQLAllocHandle(SQL_HANDLE_STMT, conn.m_handle, &m_handle); 
  assert(Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO);

#ifndef NDEBUG
  bool b = 
#endif
  set_scrollable(true);
  assert(b);
}

statement::~statement()
{
  SQLFreeHandle(SQL_HANDLE_STMT, m_handle);
}

bool statement::set_scrollable(bool scr)
{
  SQLRETURN Ret;

  Ret = SQLSetStmtAttr(m_handle, SQL_ATTR_CURSOR_SCROLLABLE,
    (SQLPOINTER)(scr?SQL_SCROLLABLE:SQL_NONSCROLLABLE), 0);
  Ret = SQLSetStmtAttr(m_handle, SQL_ATTR_CURSOR_TYPE,
    (SQLPOINTER)(scr?SQL_CURSOR_STATIC:SQL_CURSOR_FORWARD_ONLY), 0);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool statement::set_fetchsize(size_t numrow)
{
  assert(false);//dont use me yet
  SQLRETURN Ret;

  Ret = SQLSetStmtAttr(m_handle, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)numrow, 0);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool statement::execute(const std::string &sqlquery)
{
  SQLRETURN Ret;

  reset();

  Ret = SQLExecDirect(m_handle, (SQLCHAR*)sqlquery.c_str(), SQL_NTS);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

int statement::get_affected_row_count(void) const
{
  SQLRETURN Ret;
  SQLINTEGER c;

  Ret = SQLRowCount(m_handle, &c);

  if (Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO)
    return c;
  else
    return 0;
}

bool statement::prepare(const std::string &sqlquery)
{
  SQLRETURN Ret;

  Ret = SQLPrepare(m_handle, (SQLCHAR*)sqlquery.c_str(), SQL_NTS);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool statement::execute(void)
{
  SQLRETURN Ret;

  reset();

  Ret = SQLExecute(m_handle);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool statement::bind_param_int(short col, dbint *v)
{
  SQLRETURN Ret;

  // no SQL_PARAM_INPUT
  Ret = SQLBindParam(m_handle, col+1, SQL_C_LONG, SQL_INTEGER, 10, 0, v, 0);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool statement::bind_param_double(short col, dbdouble *v)
{
  SQLRETURN Ret;

  // no SQL_PARAM_INPUT
  Ret = SQLBindParam(m_handle, col+1, SQL_C_DOUBLE, SQL_DOUBLE, 10, 6, v, 0);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool statement::bind_param_string(short col, dbstring *v)
{
  SQLRETURN Ret;

  // no SQL_PARAM_INPUT
  Ret = SQLBindParam(m_handle, col+1, SQL_C_CHAR, SQL_VARCHAR, v->size()+1, 0, v->m_buf, 0);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool statement::bind_param_date(short col, dbdate *v)
{
  SQLRETURN Ret;

  Ret = SQLBindParam(m_handle, col+1, SQL_C_DATE, SQL_DATE, 0, 0, v, 0);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool statement::bind_param_timestamp(short col, dbtimestamp *v)
{
  SQLRETURN Ret;

  Ret = SQLBindParam(m_handle, col+1, SQL_C_TIMESTAMP, SQL_TIMESTAMP, 0, 0, v, 0);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool statement::close_cursor(void)
{
  // reset bind counts
  m_param_count = m_col_count = bind_start_count;

  //SQLRETURN Ret;

  //Ret = SQLCloseCursor(m_handle);
  SQLFreeStmt(m_handle, SQL_RESET_PARAMS);
  SQLFreeStmt(m_handle, SQL_UNBIND);
  SQLFreeStmt(m_handle, SQL_CLOSE);

  m_validfetch = false;

  return true;
  //return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

short statement::get_num_cols(void) const
{
  SQLSMALLINT numcol;
  SQLRETURN Ret;

  Ret = SQLNumResultCols(m_handle, &numcol);
  if (Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO)
    return numcol;
  else
    return 0;
}

bool statement::fetch_next(void)
{
  SQLRETURN Ret;

  Ret = SQLFetch(m_handle);

  row_clear();
  m_validfetch = Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;

  return m_validfetch;
}

bool statement::fetch_rownum(int id)
{
  assert(false);//dont use me yet
  SQLRETURN Ret;

  // fixme - this only works for -1, 0... make it actually do
  // what it says it does sometime?
  assert(id == 0 || id == -1);
  if (id == 0)
    Ret = SQLFetchScroll(m_handle, SQL_FETCH_FIRST, 0);
  else
    Ret = SQLFetchScroll(m_handle, SQL_FETCH_LAST, 0);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

int statement::get_current_rownum(void) const
{
  SQLRETURN Ret;
  SQLUINTEGER id;

  Ret = SQLGetStmtAttr(m_handle, SQL_ATTR_ROW_NUMBER, &id, 0, 0);
  assert( Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO );

  if (id <= 0)
    return -1;
  else
    return id-1;
}

bool statement::get_int(short col, dbint &out) const
{
  SQLRETURN Ret;
  SQLINTEGER outlen;

  Ret = SQLGetData(m_handle, col+1, SQL_C_LONG, &out, 0, &outlen);
  return (Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO) && outlen != SQL_NULL_DATA;
}

bool statement::get_double(short col, dbdouble &out) const
{
  SQLRETURN Ret;
  SQLINTEGER outlen;

  Ret = SQLGetData(m_handle, col+1, SQL_C_DOUBLE, &out, 0, 0);
  return (Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO) && outlen != SQL_NULL_DATA;
}

bool statement::get_string(short col, dbstring &out) const
{
  SQLRETURN Ret;
  SQLINTEGER outlen;

  Ret = SQLGetData(m_handle, col+1, SQL_C_CHAR, out.m_buf, out.m_max_size, &out.m_cur_size);
  if (out.m_cur_size == SQL_NULL_DATA)
    out.clear();

  return (Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO) && outlen != SQL_NULL_DATA;
}

bool statement::get_date(short col, dbdate &out) const
{
  SQLRETURN Ret;
  SQLINTEGER outlen;

  Ret = SQLGetData(m_handle, col+1, SQL_C_DATE, &out, 0, 0);

  return (Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO) && outlen != SQL_NULL_DATA;
}

bool statement::get_timestamp(short col, dbtimestamp &out) const
{
  SQLRETURN Ret;
  SQLINTEGER outlen;

  Ret = SQLGetData(m_handle, col+1, SQL_C_TIMESTAMP, &out, 0, 0);

  return (Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO) && outlen != SQL_NULL_DATA;
}

static std::string g_badcolstr("badcol"), g_nullstr("null");

bool statement::is_null(const std::string &colname) const
{
  short colnum;

  if (!find_colnum(colname, colnum))
    return false;

  if (colnum >= m_row.size() || !m_row[colnum].fetched)
    row_load(colnum);

  assert(m_row[colnum].fetched);

  return m_row[colnum].isnull;
}

const std::string & statement::operator [](const std::string &colname) const
{
  short colnum;

  if (!find_colnum(colname, colnum))
    return g_badcolstr;

  if (colnum >= m_row.size() || !m_row[colnum].fetched)
    row_load(colnum);

  assert(m_row[colnum].fetched);

  if (m_row[colnum].isnull)
    return g_nullstr;

  return m_row[colnum].value;
}

bool statement::bind_col_int(short col, dbint *v)
{
  SQLRETURN Ret;

  Ret = SQLBindCol(m_handle, col+1, SQL_C_LONG, v, 0, 0);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool statement::bind_col_double(short col, dbdouble *v)
{
  SQLRETURN Ret;

  Ret = SQLBindCol(m_handle, col+1, SQL_C_DOUBLE, v, 0, 0);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool statement::bind_col_string(short col, dbstring *v)
{
  SQLRETURN Ret;

  Ret = SQLBindCol(m_handle, col+1, SQL_C_CHAR, v->m_buf, v->m_max_size, &v->m_cur_size);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool statement::bind_col_date(short col, dbdate *v)
{
  SQLRETURN Ret;

  Ret = SQLBindCol(m_handle, col+1, SQL_C_DATE, v, 0, 0);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool statement::bind_col_timestamp(short col, dbtimestamp *v)
{
  SQLRETURN Ret;

  Ret = SQLBindCol(m_handle, col+1, SQL_C_TIMESTAMP, v, 0, 0);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool statement::find_colnum(const std::string &name, short &colnum) const
{
  short x;
  fieldnames_t::iterator ii, endii;

  if (m_fieldnames.empty())
    load_fieldnames();

  endii = m_fieldnames.end();
  for (x=0, ii=m_fieldnames.begin(); ii!=endii; ++ii, ++x)
    if (*ii == name) {
      colnum = x;
      return true;
    }

  return false;
}

const statement::fieldnames_t & statement::get_fieldnames(void) const
{
  if (m_fieldnames.empty())
    load_fieldnames();
  
  return m_fieldnames;
}

const statement::fieldsizes_t & statement::get_fieldsizes(void) const
{
  if (m_fieldnames.empty())
    load_fieldnames();
  
  return m_fieldsizes;
}

void statement::row_load(short col) const
{
  if (m_fieldnames.empty())
    load_fieldnames();

  assert(col<m_fieldnames.size());

  if (m_row.size() != m_fieldnames.size()) {
    m_row.resize(m_fieldnames.size());
    row_clear();
  }

  fixed_array<char, 256> buf;
  SQLRETURN resultcode;
  SQLINTEGER outlen;

  for (short c=0; c<=col; ++c) {
    colval_t &r = m_row[c];

    if (r.fetched)
      continue;

    r.fetched = true;

    // I really want to use SQL_C_BINARY here... however, it seems that the odbc layer
    // (or postgresql odbc driver) actually _interprets_ stuff when decoding the string
    // for example, if the data is of type varchar or test, then \ of all things will
    // actually cause all kinda of funky stuff... hopefully this will be figured out
    // in the future.
    //
    // For now, use CHAR... should be fine until we want to do binaries and stuff

    //resultcode = SQLGetData(m_handle, c+1, SQL_C_BINARY, buf.c_array(), buf.size(), &outlen);
    resultcode = SQLGetData(m_handle, c+1, SQL_C_CHAR, buf.c_array(), buf.size(), &outlen);

    if (resultcode != SQL_SUCCESS && resultcode != SQL_SUCCESS_WITH_INFO) {
      r.isnull = true;    // what else can we do?
      continue;
    }

    if (outlen == SQL_NULL_DATA) {
      r.isnull = true;
      continue;
    }

    r.isnull = false;
    r.value.clear();

    if (resultcode == SQL_NO_DATA)
      continue;

    assert(outlen != SQL_NO_TOTAL);

    //SQL_C_BINARY version
    //if (outlen > buf.size())
      //outlen = buf.size();
    //SQL_C_CHAR version
    if (outlen >= buf.size())
      outlen = buf.size() - 1;

    r.value.assign(buf.c_array(), outlen);

    while (resultcode == SQL_SUCCESS_WITH_INFO) {
      //resultcode = SQLGetData(m_handle, c+1, SQL_C_BINARY, buf.c_array(), buf.size(), &outlen);
      resultcode = SQLGetData(m_handle, c+1, SQL_C_CHAR, buf.c_array(), buf.size(), &outlen);

      if (resultcode == SQL_NO_DATA)
        break;

      assert(outlen != SQL_NO_TOTAL);

      //SQL_C_BINARY version
      //if (outlen > buf.size())
        //outlen = buf.size();
      //SQL_C_CHAR version
      if (outlen >= buf.size())
        outlen = buf.size() - 1;

      r.value.append(buf.c_array(), outlen);
    }
  } //for
}

void statement::row_clear(void) const
{
  for (short x=0; x<m_row.size(); ++x)
    m_row[x].fetched = false;
}

void statement::reset(void)
{
  m_fieldnames.clear();
  m_fieldsizes.clear();
}

void statement::load_fieldnames(void) const
{
  short x, w;
  dbstring buf;
  SQLRETURN Ret;
  SQLSMALLINT dtype, dec, nl, len;
  SQLUINTEGER colsize;

  w = get_num_cols();
  // w == 0 is ok, i guess

  std::string tempstring;

  // fill in the buffer
  m_fieldnames.resize(w);
  m_fieldsizes.resize(w);

  for (x=0; x<w; ++x) {
    Ret = SQLDescribeCol(m_handle, x+1, buf.m_buf, buf.m_max_size, &len,
      &dtype, &colsize, &dec, &nl);
    buf.m_cur_size = len;
    if (Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO)
    {
      buf.str(tempstring);
      // convert fields to lowercase
      lowercase(tempstring, m_fieldnames[x]);
      // save the size
      m_fieldsizes[x] = colsize;
    } else
      m_fieldsizes[x] = 0;
  }
}

