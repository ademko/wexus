
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

#include <wexus/db/connection.h>

#include <scopira/tool/output.h>
#include <wexus/db/statement.h>

//BBlibs -lodbc
//BBtargets libwexus.so

using namespace wexus;

db::env::env(void)
{
  SQLRETURN Ret;

  // might aswell assert this here
  assert(sizeof(SQLUINTEGER) == sizeof(int));
  assert(sizeof(SQLINTEGER) == sizeof(int));
  assert(sizeof(SQLDOUBLE) == sizeof(double));

  Ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_handle);
  assert(Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO);
  assert(m_handle);

  Ret = SQLSetEnvAttr(m_handle, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0); 
  assert(Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO);
}

db::env::~env(void)
{
  assert(is_alive_object());
  SQLFreeHandle(SQL_HANDLE_ENV, m_handle);
}

db::connection::connection(env &e)
{
  SQLRETURN Ret;

  OUTPUT << "[ODBC1";
  Ret = SQLAllocHandle(SQL_HANDLE_DBC, e.m_handle, &m_handle); 
  assert(Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO);
  Ret = SQLSetConnectAttr(m_handle, SQL_LOGIN_TIMEOUT, (SQLPOINTER)(5), 0);
  assert(Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO);
  //Ret = SQLSetConnectAttr(m_handle, SQL_ATTR_ACCESS_MODE, (SQLPOINTER)(SQL_MODE_READ_WRITE), 0);
  //assert(Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO);
  assert(m_handle);
}

db::connection::~connection()
{
  SQLFreeHandle(SQL_HANDLE_DBC, m_handle);
}

bool db::connection::connect(const std::string &DSN, const std::string &userid, const std::string &pw)
{
  SQLRETURN Ret;

  assert(m_handle);

  assert(&DSN);
  assert(&userid);
  assert(&pw);
  assert(DSN.size()>0);
  assert(userid.size()>0);

  OUTPUT << "2";
  // these casts are blasphemous
  Ret = SQLConnect(m_handle, (SQLCHAR*)DSN.c_str(), SQL_NTS, (SQLCHAR*)userid.c_str(), SQL_NTS, (SQLCHAR*)pw.c_str(), SQL_NTS);
  OUTPUT << "3]";

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool db::connection::set_autocommit(bool autoc)
{
  SQLRETURN Ret;

  Ret = SQLSetConnectAttr(m_handle, SQL_ATTR_AUTOCOMMIT,
    (SQLPOINTER)(autoc?SQL_AUTOCOMMIT_ON:SQL_AUTOCOMMIT_OFF), 0);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool db::connection::commit(void)
{
  SQLRETURN Ret;

  Ret = SQLEndTran(SQL_HANDLE_DBC, m_handle, SQL_COMMIT);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

bool db::connection::rollback(void)
{
  SQLRETURN Ret;

  Ret = SQLEndTran(SQL_HANDLE_DBC, m_handle, SQL_ROLLBACK);

  return Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO;
}

db::statement * db::connection::new_statement(void)
{
  return new db::statement(*this);
}

