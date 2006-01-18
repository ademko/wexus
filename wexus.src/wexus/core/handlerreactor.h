
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

#ifndef __INCLUDED_WEXUS_CORE_HANDLERREACTOR_HPP__
#define __INCLUDED_WEXUS_CORE_HANDLERREACTOR_HPP__

#include <string>
#include <map>

#include <scopira/tool/object.h>

#include <wexus/core/engine.h>
#include <wexus/core/app.h>
#include <wexus/core/reactor.h>
#include <wexus/core/handler.h>
#include <wexus/core/recmanager.h>

#include <wexus/rec/memrec.h>

#include <wexus/db/connection.h>

namespace wexus
{
  namespace db
  {
    // forward
    class connection_pool;
  }
  namespace core
  {
    class handler_event_data;
    class handler_event;
    class handler_reactor;
    // forward
    class session_manager;
    class session_i;
  }
}

/**
 * additional handler data
 *
 * @author Aleksander Demko
 */
class wexus::core::handler_event_data
{
  public:
    /// ctor, p can be null
    handler_event_data(db::connection_pool *p);
    /// dtor
    ~handler_event_data();

    /// sets the filename of the rec template
    void set_template(const std::string &name)
      { m_temname = name; }
    const std::string & get_template(void) const
      { return m_temname; }

    wexus::rec::mem_rec & get_root(void)
      { return m_rootrec; }

    /// adds a error msg to the root
    void add_error(const std::string &msg);
    /// adds a error msg to the root, with a name
    void add_error(const std::string &name, const std::string &msg);
    /// has any errors?
    bool has_error(void);

    /// get the database connect, if any
    db::connection * get_connection(void);

  private:
    std::string m_temname;
    wexus::rec::mem_rec m_rootrec;
    db::connection_pool *m_conn_pool;     /// might be null
    scopira::tool::count_ptr< wexus::db::connection > m_cur_conn;

    /// im notcopyable
    handler_event_data(const handler_event_data&);
    /// or assigable
};

/**
 * a handle event
 *
 * @author Aleksander Demko
 */
class wexus::core::handler_event : public wexus::core::app_event
{
  public:
    handler_event_data &handler;
    session_i *session;       /// pointer, cuz it might be null

    /// initing ctor
    handler_event(const std::string &reqe, front_event_i &fronte,
      app_event_data &appe, handler_event_data &handlere,
      session_i *ses)
      : app_event(reqe, fronte, appe), handler(handlere), session(ses) { }

    /// nice routine
    wexus::rec::mem_rec & root(void) { return handler.get_root(); }
    /// nice routine
    db::connection & db(void) { return *handler.get_connection(); }
    /// nice routine
    void set_template(const std::string &name) { handler.set_template(name); }
    /// checkers
    checkers & check(void) { return engine::instance()->get_checkers(); }

    /// checker test (returns c)
    bool test_check(const std::string &msg, bool c) { if (!c) handler.add_error(msg); return c; }
    /// checker test, with errorname support (returns c)
    bool test_check(const std::string &msgname, const std::string &msg, bool c) { if (!c) handler.add_error(msgname, msg); return c; }

    /// has_error redirect
    bool has_error(void) { return handler.has_error(); }

    /// sets the session -- ses may be null to indicate a clear operation
    void set_session(session_manager *sesman, session_i *ses);

    // default copy ctor and dtor are fine.
};

/**
 * reactor interface. most user-apps will use the prebuild, supplied
 * reactors in core. in certain specialized cases they will also
 * make their own
 *
 * @author Aleksander Demko
 */
class wexus::core::handler_reactor : public virtual wexus::core::reactor_i
{
  public:
    /// ctor, sesmgr can be null, dbpool mgr can be null
    handler_reactor(rec_manager *mgr, session_manager *sesmgr = 0,
      db::connection_pool *dbpoolmgr = 0);

    /**
     * called when its time to handle an event
     *
     * this version goes through the reactor stack, looking for
     * a reactor to handle this event
     *
     * @return true if this reactor handled the event
     * @author Aleksander Demko
     */ 
    virtual bool handle_app_event(app_event &evt);

    /**
     * adds a handler to this handlerreactor's handler map
     *
     * @author Aleksander Demko
     */ 
    void add_handler(const std::string &req, handler_i* handler);

  protected:
    typedef std::map<std::string, scopira::tool::count_ptr<handler_i> > handlermap_t;
    
    wexus::core::rec_manager *m_mgr;    /// the manager
    handlermap_t m_handlermap; /// the map of handlers

    session_manager *m_sesmgr;      /// manager o sessions, might be null
    db::connection_pool *m_dbpoolmgr;    /// manager o db connections, might be null

    void post_template(handler_event &evt);
};

#endif

