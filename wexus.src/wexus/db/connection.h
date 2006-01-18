
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

#ifndef __INCLUDED_WEXUS_DB_CONNECTION_HPP__
#define __INCLUDED_WEXUS_DB_CONNECTION_HPP__

#include <wexus/db/dbtypes.h>

namespace wexus
{
  /**
   * This subsytem provides light ODBC wrappers for SQL database access
   * @author Aleksander Demko
   */ 
  namespace db
  {
    class env;
    class connection;
    class statement;//fwd
  }
}

/**
 * and ODBC envirotment. other to all handles
 *
 * @author Aleksander Demko
 */ 
class wexus::db::env : public wexus::db::handle
{
  public:
    /// ctor
    env(void);
    /// dtor
    virtual ~env();

    /// gets the handle type
    virtual char get_type(void) const { return SQL_HANDLE_ENV; }

    friend class connection;
};

/**
 * an ODBC database connection
 *
 * @author Aleksander Demko
 */
class wexus::db::connection : public wexus::db::handle
{
  public:
    /// ctor
    explicit connection(env &e);
    /// dtor
    virtual ~connection();
  
    /// gets the handle type
    virtual char get_type(void) const { return SQL_HANDLE_DBC; }

    /**
     * login, returns true on success
     *
     * @author Aleksander Demko
     */
    bool connect(const std::string &DSN, const std::string &userid, const std::string &pw);

    // technically, these should also be in env, but we'll probably never use em there

    /// set the autocommits mode (by default, on)
    bool set_autocommit(bool autoc);

    /// commit the current transaction
    bool commit(void);
    /// rollback the current transaction
    bool rollback(void);


    // convience routine, does new statement(this) (hense, caller must clean up)
    statement * new_statement(void);

    friend class statement;
};

#endif

