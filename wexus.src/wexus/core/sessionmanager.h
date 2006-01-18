
/*
 *  Copyright (c) 2004-2006    Aleksander B. Demko
 *
 *  All rights reserved.
 *
 *  This material is confidential and proprietary information of
 *  Aleksander B. Demko ("Confidential Information").
 *  This Confidential Information may only be used and reproduced
 *  in accordance with the terms of the license agreement.
 *
 */

#ifndef __INCLUDED__WEXUS_CORE_SESSIONMANAGER_HPP__
#define __INCLUDED__WEXUS_CORE_SESSIONMANAGER_HPP__

#include <map>
#include <string>

#include <scopira/tool/thread.h>
#include <scopira/tool/time.h>

namespace scopira
{
  namespace tool
  {
    class property;
  }
}

namespace wexus
{
  namespace core
  {
    class session_i;

    class session_manager;
    // forward
    class front_event_i;
  }
}

/**
 * base for all session objects
 *
 * @author Aleksander Demko
 */
class wexus::core::session_i : public virtual scopira::tool::object
{
  public:
    /// get the unique id for this session
    int get_sessionid(void) const;

    virtual bool load(scopira::tool::iobjflow_i& in);
    virtual void save(scopira::tool::oobjflow_i& out) const;
    
    /**
     * called when the session is about to be nuked, decendant classes
     * should overide this. default implementaion does nothing.
     *
     * @author Aleksander Demko
     */
    virtual void prune(bool properlogoff);

  protected:
    /// ctor
    session_i(void);

  private:
    // only session manager will call these routines

    /// sets the session id of this object
    void set_sessionid(int sesid) { m_sessionid = sesid; }

    /// touch my stamp
    void touch_stamp(void);

    /// get my time stamp
    scopira::tool::timestamp get_stamp(void) const;

    friend class session_manager;

  private:
    mutable scopira::tool::rwlock m_stamprw;
    int m_sessionid;       /// the session id
    scopira::tool::timestamp m_stamp; /// time stamp
};

/**
 * a manager of sessions
 *
 * @author Aleksander Demko
 */
class wexus::core::session_manager
{
  public:
    /// ctor
    session_manager(void);
    /// ctor
    session_manager(const std::string &cookiename, const std::string &cookiepath, const std::string &cookiedomain);
    /// dtor
    ~session_manager();


    /// default-ctor targetted init
    bool init_prop(scopira::tool::property *prop);

    /// adds the given session. it will be ref counted
    void set_session(session_i *ses);
    /// remove the given session
    void clear_session(int id);

    /// gets the session, null if not found
    session_i * get_session(int id);

    /// gets the session for the event, if any.
    /// returns null on not needed
    session_i * get_session(front_event_i &he);

    /// sets the session, aswel as informing the client
    /// this can be used for login/logout
    /// null ses indicates clear
    /// this one should be refactoed away
    void set_session(front_event_i &he, session_i *ses);
    void clear_session(front_event_i &he, int id);

    /**
     * called once in awhile to clean up all stale
     * sessions. this method should be added to a tick manager
     *
     * @author Aleksander Demko
     */ 
    void prune(void);

    /**
     * flushes all current sessions, regardless of time
     *
     * @author Aleksander Demko
     */
    void prune_all(void);

    /**
     * Enables supplementale storing to the file system.
     * This is useful for debugging, mostly.
     * empty basedir disables this facility.
     *
     * @param basedir is a base directory, for example, /tmp
     * @author Aleksander Demko
     */ 
    void set_file_store(const std::string &basedir);

    // set_db_store in the future?

  private:
    typedef std::map<int, scopira::tool::count_ptr< session_i > > sesmap_t;

    scopira::tool::rwlock m_seslock;
    sesmap_t m_sessions;

    std::string m_cookiename;         /// name of the cookie to extract
    std::string m_cookiepath;
    std::string m_cookiedomain;

    std::string m_filestore;          /// optional, "" if not in use

  private:
    int next_id(void) const;  //asumers lock is locked
};

#endif

