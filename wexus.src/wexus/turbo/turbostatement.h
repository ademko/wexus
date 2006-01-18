
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

#ifndef __INCLUDED_WEXUS_TURBO_AUTOSTATEMENT_H__
#define __INCLUDED_WEXUS_TURBO_AUTOSTATEMENT_H__

#include <wexus/db/statement.h>

namespace wexus
{
  namespace turbo
  {
    class turbo_statement;
    class turbo_xtion;
  }
}

/**
 * A statement like thing that can use streams to build itself.
 *
 * Basically, you use this statement-decent. Rather than do a prepare() plus binds,
 * do a s << outputparams << "query" << inputparams << query << s.endl().
 *
 * Then execute/fetch as normal.
 *
 * Use the reset() thing if you wan to reuse this statemenet after.
 *
 * @author Aleksander Demko
 */ 
class wexus::turbo::turbo_statement : public wexus::db::statement
{
  public:
    /// default ctor
    turbo_statement(void);
    /**
     * Ctor
     * @author Aleksander Demko
     */
    explicit turbo_statement(wexus::db::connection &conn);

    turbo_statement & operator << (wexus::db::dbstring &v);
    turbo_statement & operator << (wexus::db::dbint &v);
    turbo_statement & operator << (const char *c);
    turbo_statement & operator << (char c);

    /// finally, do the prepare. you must call one of the prepare*f variants
    static char preparef(void) { return 1; }
    /// does a prepare, then execute
    static char prepare_executef(void) { return 3; }
    /// does a prepare, then execute and fetch
    static char prepare_execute_fetchf(void) { return 4; }

    /**
     * This one builds on statement::operator[]. It will return on the same things,
     * EXCEPT if the column doesn't exist, a t_assert-like exception is thrown.
     *
     * @author Aleksander Demko
     */ 
    const std::string & operator [](const std::string &colname) const;

  private:
    std::string m_query;
    bool m_fresh;

  private:
    void internal_reset(void);
};

/**
 * A transaction.
 *
 * @author Aleksander Demko
 */ 
class wexus::turbo::turbo_xtion
{
  public:
    /// ctor
    turbo_xtion(bool defaultcommit);    // what would be the reasonable default? who knows, both are valid
    /// dtor
    ~turbo_xtion();

    /**
     * Override the default handling and simply set weather (upon destruction)
     * the transaction will be commited (true) or rolled back (false)
     *
     * @author Aleksander Demko
     */ 
    void set_commit(bool b) { m_userset = true; m_commit = b; }

    /// similar
    void commit_xtion(void) { set_commit(true); }

  private:
    bool m_userset, m_commit;

    wexus::db::connection &db;
};

#endif

