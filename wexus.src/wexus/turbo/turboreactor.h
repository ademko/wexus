
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

#ifndef __INCLUDED_WEXUS_TURBO_TURBOREACTOR_H__
#define __INCLUDED_WEXUS_TURBO_TURBOREACTOR_H__

#include <wexus/core/reactor.h>
#include <wexus/core/app.h>
#include <wexus/core/front.h>
#include <wexus/core/sessionmanager.h>
#include <wexus/db/pool.h>

namespace wexus
{
  namespace turbo
  {
    class string_iterator;

    class turbo_event_data;
    class turbo_event;
    class turbo_reactor;

    turbo_event & get_tls_event(void);

    class registry;   //fwd
  }
}

class wexus::turbo::string_iterator
{
  public:
    typedef std::multimap<std::string, std::string>::const_iterator iterator_type;

  public:
    string_iterator(void) { }
    string_iterator(iterator_type b, iterator_type e) : m_b(b), m_e(e) { }

    bool is_valid(void) const { return m_b != m_e; }
  
    const std::string & operator *(void) const { return m_b->second; }

    void operator ++(void) { ++m_b; }

  private:
    iterator_type m_b, m_e;
};

/**
 * Internal payload for turbo_event
 *
 * @author Aleksander Demko
 */ 
class wexus::turbo::turbo_event_data
{
  public:
    /// ctor, p can be null
    turbo_event_data(core::session_manager *sesmgr, wexus::db::connection_pool *p);
    /// dtor
    ~turbo_event_data();

    /// gets the session manager, might be null
    core::session_manager * get_session_manager(void) const { return m_sesmgr; }
    /// get the database connect, if any
    db::connection * get_connection(void);

    /// adds a error msg to the root
    void add_error(const std::string &msg);
    /// adds a error msg to the root, with a name
    void add_error(const std::string &name, const std::string &msg);
    /// has any string_iterator?
    bool has_error(void) const { return !m_errors.empty(); }
    /// gets the current set of string_iterator
    string_iterator get_errors(void) const;
    /// gets the current set of string_iterator with the given name
    string_iterator get_errors(const std::string &key) const;
    /// clears all the string_iterator
    void clear_errors(void);

    /// adds a note msg to the root
    void add_note(const std::string &msg);
    /// adds a note msg to the root, with a name
    void add_note(const std::string &name, const std::string &msg);
    /// has any string_iterator?
    bool has_note(void) const { return !m_notes.empty(); }
    /// gets the current set of string_iterator
    string_iterator get_notes(void) const;
    /// gets the current set of string_iterator with the given name
    string_iterator get_notes(const std::string &key) const;
    /// clears all the string_iterator
    void clear_notes(void);

    bool has_formcache_field(const std::string &fieldname) const { return m_formcache.count(fieldname) == 1; }
    /// can fail!
    const std::string & formcache_field(const std::string &fieldname) const { return m_formcache.find(fieldname)->second; }
    /// never fails
    std::string & formcache_field(const std::string &fieldname) { return m_formcache[fieldname]; }

  private:
    core::session_manager *m_sesmgr;
    db::connection_pool *m_conn_pool;     /// might be null
    scopira::tool::count_ptr< wexus::db::connection > m_cur_conn;
    typedef std::multimap<std::string, std::string> stringmap_t;
    stringmap_t m_errors, m_notes;
    typedef std::map<std::string, std::string> formcache_t;
    formcache_t m_formcache;
};

/**
 * A turbo event
 *
 * @author Aleksander Demko
 */
class wexus::turbo::turbo_event : public wexus::core::app_event
{
  public:
    class html_encoder_flow : public scopira::tool::oflow_i
    {
      private:
        scopira::tool::count2_ptr<scopira::tool::oflow_i> m_out;
      public:
        html_encoder_flow(bool doref, scopira::tool::oflow_i *out);
        virtual bool failed(void) const { return m_out->failed(); }
        virtual size_t write(const byte_t* _buf, size_t _size);
    };
  public:
    turbo_event_data &turbo;
    wexus::core::session_i *session;       /// pointer, cuz it might be null
    scopira::tool::count_ptr<html_encoder_flow> html_encoder;
  public:
    turbo_event(const std::string &reqe, wexus::core::front_event_i &fronte, wexus::core::app_event_data &appe,
      turbo_event_data &turboe, wexus::core::session_i *ses)
      : app_event(reqe, fronte, appe), turbo(turboe), session(ses) { }

    /// nice routine
    db::connection & db(void) { return *turbo.get_connection(); }
    /// sets the session -- ses may be null to indicate a clear operation
    void set_session(wexus::core::session_i *ses);
    /// main output handler
    scopira::tool::oflow_i & output(void) { return front.get_output(); }
    /// output handler, that will encode stuff to be safe for html
    scopira::tool::oflow_i & html_output(void);

    /// adds a error msg to the root
    void add_error(const std::string &msg) { turbo.add_error(msg); }
    /// adds a error msg to the root, with a name
    void add_error(const std::string &name, const std::string &msg) { turbo.add_error(name, msg); }
    /// has any errors?
    bool has_error(void) const { return turbo.has_error(); }
    /// gets the current set of errors
    string_iterator get_errors(void) const { return turbo.get_errors(); }
    /// gets the current set of errors with the given name
    string_iterator get_errors(const std::string &key) const { return turbo.get_errors(key); }
    /// clears all the errors
    void clear_errors(void) { turbo.clear_errors(); }

    /// this version queues a note FOR THE NEXT page
    /// this requires a session of atleast type turbo::user_session
    void add_next_note(const std::string &msg);
    /// this version queues a note FOR THE NEXT page
    /// this requires a session of atleast type turbo::user_session
    void add_next_note(const std::string &name, const std::string &msg);

    /// adds a note msg to the root
    void add_note(const std::string &msg) { turbo.add_note(msg); }
    /// adds a note msg to the root, with a name
    void add_note(const std::string &name, const std::string &msg) { turbo.add_note(name, msg); }
    /// has any notes?
    bool has_note(void) const { return turbo.has_note(); }
    /// gets the current set of notes
    string_iterator get_notes(void) const { return turbo.get_notes(); }
    /// gets the current set of notes with the given name
    string_iterator get_notes(const std::string &key) const { return turbo.get_notes(key); }
    /// clears all the notes
    void clear_notes(void) { turbo.clear_notes(); }

    /// gets the field from the cache or actual form. always works (returns "" on not found)
    const std::string & get_form_field(const std::string &fieldname) const;
    /// checks if the form field exists, either in the cache or the form
    bool has_form_field(const std::string& name) const;
    /// sets the form field value in the cache
    void set_form_field(const std::string &fieldname, const std::string &val);
    /// sets the form field value in the cache
    void set_form_field_default(const std::string &fieldname, const std::string &val);

    /// nice helper
    const std::string & operator [](const std::string &fieldname) const { return get_form_field(fieldname); }
    /// nice helper... need to fix this up first
    /// no, i dont think a non-const version of this will ever be safe
    //std::string & operator [](const std::string &fieldname) { return turbo.formcache_field(fieldname).first; }
};

/**
 * A reactor that dispatches
 * calls to the turbo_registry, if they have
 *
 * @author Aleksander Demko
 */
class wexus::turbo::turbo_reactor : public virtual wexus::core::reactor_i
{
  public:
    /// ctor, dbpool mgr can be null
    turbo_reactor(core::session_manager *sesmgr, db::connection_pool *dbpoolmgr = 0);
    /// dtor
    virtual ~turbo_reactor();

    /**
     * called when its time to handle an event
     *
     * this version goes through the reactor stack, looking for
     * a reactor to handle this event
     *
     * @return true if this reactor handled the event
     * @author Aleksander Demko
     */ 
    virtual bool handle_app_event(core::app_event &evt);

  protected:
    core::session_manager *m_sesmgr;      /// manager o sessions
    db::connection_pool *m_dbpoolmgr;    /// manager o db connections, might be null
    registry *m_reg;
};

#endif

