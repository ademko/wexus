
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

#ifndef __INCLUDED_WEXUS_DB_STATEMENT_HPP__
#define __INCLUDED_WEXUS_DB_STATEMENT_HPP__

#include <vector>

#include <wexus/db/dbtypes.h>

namespace wexus
{
  namespace db
  {
    class connection; //fwd
    class statement;
    typedef scopira::tool::count_ptr< statement > statement_ptr;
  }
}

/**
 * An ODBC Database statement. Because of ODBC, this statement is also highly restrictive.
 *
 * Always bind your input values, if any, in increasing column order.
 *
 * Data extraction can be done in one of 3 ways (to not mix these methods for the same query):
 *  - output binding (remember to bind in increasing order.
 *    Recommended if you must programmically process the data.
 *  - use is_null/operator[]. You can call this in any order, and as many times as you want,
 *    as it does its own caching. Recommended for random data access especially for the
 *    purposes of screen output.
 *  - get_ calls. Restrictive. get_ in increasing col order. Don't col get on the same col
 *    more than once. Not really recommended.
 *
 * Consider use auto_statement to help with input binding (and if you choose to use output binding)
 * output binding.
 *
 * Rec only: the *db_rec classes do their own buffering to hide these restrictions
 *
 * @author Aleksander Demko
 */
class wexus::db::statement : public wexus::db::handle
{
  public:
    typedef std::vector< std::string > fieldnames_t;
    typedef std::vector< size_t > fieldsizes_t;

  public:
    /// ctor, with explicit connection init
    explicit statement(connection &conn);
    /// dtor
    virtual ~statement();

    /// gets the handle type
    virtual char get_type(void) const { return SQL_HANDLE_STMT; }

    // executation stuff
    // all funcs return true on success

    /// should the results be scrollable? (by default, they ARE)
    bool set_scrollable(bool scr);

    /// set the size of the fetch buffer (default 0)
    /// dunno if this is what i thought it to be - dont call this yet
    bool set_fetchsize(size_t numrow);

    /// execute a direct, one time statement. true on success
    bool execute(const std::string &sqlquery);

    /// prepare a statement
    bool prepare(const std::string &sqlquery);
    /// execute the prepared statement
    bool execute(void);

    /// combined method
    bool execute_fetch(void) { return execute() && fetch_next(); }

    /// are we currently at a valid row (ie. the last fetch_next succeeded)
    bool is_valid(void) const { return m_validfetch; }

    /// returns the number of rows affected by an insert/update/delete execute()
    int get_affected_row_count(void) const;

    /// bind a param (0-based)
    bool bind_param_int(short col, dbint *v);
    /// bind a param (0-based)
    bool bind_param_double(short col, dbdouble *v);
    /// bind a param (0-based)
    bool bind_param_string(short col, dbstring *v);
    /// bind a param (0-based)
    bool bind_param_date(short col, dbdate *v);
    /// bind a param (0-based)
    bool bind_param_timestamp(short col, dbtimestamp *v);

    /// bind a param (each bind_param call increases the current column to bind to)
    bool bind_param_int(dbint *v)             { return bind_param_int(m_param_count++, v); }
    /// bind a param (each bind_param call increases the current column to bind to)
    bool bind_param_double(dbdouble *v)       { return bind_param_double(m_param_count++, v); }
    /// bind a param (each bind_param call increases the current column to bind to)
    bool bind_param_string(dbstring *v)       { return bind_param_string(m_param_count++, v); }
    /// bind a param (each bind_param call increases the current column to bind to)
    bool bind_param_date(dbdate *v)       { return bind_param_date(m_param_count++, v); }
    /// bind a param (each bind_param call increases the current column to bind to)
    bool bind_param_timestamp(dbtimestamp *v)       { return bind_param_timestamp(m_param_count++, v); }

    /// close the cursor, should be called before reissuing any executes()
    bool close_cursor(void);
    /// nicer lookin
    bool close(void) { return close_cursor(); }

    // result set stuff

    /// gets the number 
    short get_num_cols(void) const;
    /// fetches the next row of datum in the result set, true on has data
    bool fetch_next(void);
    /// fetch a specific row, 0==first, -1 is an alias for last
    bool fetch_rownum(int id);

    /// current the current row num (0-based), -1 on "no current row"
    int get_current_rownum(void) const;

    /// data access, col # is 0-based
    bool get_int(short col, dbint &out) const;
    /// data access, col # is 0-based
    bool get_double(short col, dbdouble &out) const;
    /// data access, col # is 0-based
    bool get_string(short col, dbstring &out) const;
    /// data access, col # is 0-based
    bool get_date(short col, dbdate &out) const;
    /// data access, col # is 0-based
    bool get_timestamp(short col, dbtimestamp &out) const;

    /// data access
    /*bool get_int(const std::string &colname, dbint &out) const;
    /// data access
    bool get_double(const std::string &colname, dbdouble &out) const;
    /// data access
    bool get_string(const std::string &colname, dbstring &out) const;
    /// data access
    bool get_date(const std::string &colname, dbdate &out) const;
    /// data access
    bool get_timestamp(const std::string &colname, dbtimestamp &out) const;

    /// data access (super safe, always works)
    dbint get_int(const std::string &colname) const;
    /// data access (super safe, always works)
    dbdouble get_double(const std::string &colname) const;
    /// data access (super safe, always works)
    dbstring get_string(const std::string &colname, int stringbufsize = 256) const;
    /// data access (super safe, always works)
    dbdate get_date(const std::string &colname) const;
    /// data access (super safe, always works)
    dbtimestamp get_timestamp(const std::string &colname) const;*/

    /// nice data access routine (super safe, always works)
    bool is_null(const std::string &colname) const;
    /// nice data access routine (super safe, always works)
    const std::string & operator [](const std::string &colname) const;

    /// bind cols (0-based)
    bool bind_col_int(short col, dbint *v);
    /// bind cols (0-based)
    bool bind_col_double(short col, dbdouble *v);
    /// bind cols (0-based)
    bool bind_col_string(short col, dbstring *v);
    /// bind cols (0-based)
    bool bind_col_date(short col, dbdate *v);
    /// bind cols (0-based)
    bool bind_col_timestamp(short col, dbtimestamp *v);

    /// bind cols (each bind_col call increases the current column to bind to)
    bool bind_col_int(dbint *v)             { return bind_col_int(m_col_count++, v); }
    /// bind cols (each bind_col call increases the current column to bind to)
    bool bind_col_double(dbdouble *v)       { return bind_col_double(m_col_count++, v); }
    /// bind cols (each bind_col call increases the current column to bind to)
    bool bind_col_string(dbstring *v)       { return bind_col_string(m_col_count++, v); }
    /// bind cols (each bind_col call increases the current column to bind to)
    bool bind_col_date(dbdate *v)       { return bind_col_date(m_col_count++, v); }
    /// bind cols (each bind_col call increases the current column to bind to)
    bool bind_col_timestamp(dbtimestamp *v)       { return bind_col_timestamp(m_col_count++, v); }

    /// find the given colnum for the given string, false if failed
    bool find_colnum(const std::string &name, short &colnum) const;

    /// gets the list of fieldnames
    const fieldnames_t & get_fieldnames(void) const;

    /// gets the list of fieldsizes
    const fieldsizes_t & get_fieldsizes(void) const;

  private:
    void row_load(short col) const;
    void row_clear(void) const;
    void reset(void);
    void load_fieldnames(void) const;

  private:
    mutable fieldnames_t m_fieldnames; //initially 0
    mutable fieldsizes_t m_fieldsizes; //initially 0

    static const int bind_start_count = 0;

    /// bind counts
    int m_param_count;
    int m_col_count;

    bool m_validfetch;

    struct colval_t {
      bool fetched;
      bool isnull;
      std::string value;

      colval_t(void) : fetched(false), isnull(false) { }
    };
    typedef std::vector<colval_t> row_t;

    mutable row_t m_row;
};

#endif

