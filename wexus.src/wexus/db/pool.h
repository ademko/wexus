
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

#ifndef __INCLUDED_WEXUS_DB_POOL_HPP__
#define __INCLUDED_WEXUS_DB_POOL_HPP__

#include <vector>

#include <scopira/tool/thread.h>

#include <wexus/db/connection.h>

namespace scopira
{
  namespace tool
  {
    class property;//fwd
  }
}

namespace wexus
{
  namespace db
  {
    class connection_pool;
    class connection;//fwd
  }
}

/**
 * a fixed size, db connection pool
 *
 * @author Aleksander Demko
 */
class wexus::db::connection_pool
{
  public:
    /// ctor, you must call one of the init methods!
    connection_pool(void);
    /// dtor
    ~connection_pool();

    /// default-ctor targetted init
    bool init_prop(scopira::tool::property *prop);
    /// init explicitly, not by propo
    bool init_pool(int conncount, const std::string &dsn, const std::string &username, const std::string &pw);

    /// pop a connection into the given ptr
    void pop(scopira::tool::count_ptr< connection > &conn);
    /// push the given connection back to the pool stack
    void push(const scopira::tool::count_ptr< connection > &conn);

  private:
    typedef std::vector< scopira::tool::count_ptr< connection > > connstack_t;

    struct shared_t {
      shared_t() { }
      shared_t(shared_t& rhs) { assert(false); }

      connstack_t connstack;
      size_t next;              /// index of next free conn
    };

    wexus::db::env m_env; /// this must be declared before the connection pool data
    scopira::tool::event_area<shared_t> m_connpool;
};

#endif

