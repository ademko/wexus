
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

#ifndef __INCLUDED_WEXUS_UI_FORMREACTOR_HPP__
#define __INCLUDED_WEXUS_UI_FORMREACTOR_HPP__

#include <wexus/core/app.h>
#include <wexus/core/sessionmanager.h>
#include <wexus/ui/element.h>

namespace wexus
{
  namespace db
  {
    // forward
    class connection_pool;
  }
  namespace ui
  {
    class form_session;
    class form_factory_i;
    class form_reactor;
  }
}

/**
 * A session that has a form object.
 * @author Aleksander Demko
 */ 
class wexus::ui::form_session : virtual public wexus::core::session_i
{
  private:
    typedef std::pair<scopira::tool::count_ptr<element>, scopira::tool::count_ptr<element> > epair_t;
    typedef std::list<epair_t> pairs_t;   // set of whats current (new), and what it replaced (old)

    scopira::tool::mutex m_element_lock;
    scopira::tool::count_ptr<element> m_cur_element;   // protected by m_element_lock
    pairs_t m_pairs;    // protected by m_element_lock

  public:
    /// ctor
    form_session(void);
    /**
     * handles a form event.
     * return false if there was to element to hand this off too
     * @author Aleksander Demko
     */
    bool handle_form_event(form_event &ev, scopira::tool::oflow_i &out);
    /**
     * Sets the current element, replacing the previous one, if
     * any. Null is ok. Will be count_ptr'ed.
     * @author Aleksander Demko
     */
    void set_current_element(element *el);

  private:
    /// internal
    void element_reparent(element *olde, element *newe);
};

/**
 * Maker of form_session.
 * Only really needed to create the first one.
 * @author Aleksander Demko
 */
class wexus::ui::form_factory_i
{
  public:
    virtual ~form_factory_i() { }
  public:
    /// generates and returns a form on the heap
    virtual wexus::ui::element * make_form(void) = 0;
};

/**
 * A reactor that handles form_session.
 * @author Aleksander Demko
 */ 
class wexus::ui::form_reactor : public virtual wexus::core::reactor_i
{
  public:
    /// ctor, dbpool mgr can be null
    form_reactor(core::session_manager *sesmgr, form_factory_i *fact, db::connection_pool *dbpoolmgr = 0);

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

    /**
     * Sets the trigger name.
     * The default is "form"
     * @author Aleksander Demko
     */ 
    void set_trigger_name(const std::string &name);

  protected:
    core::session_manager *m_sesmgr;      /// manager o sessions
    db::connection_pool *m_dbpoolmgr;    /// manager o db connections, might be null

    form_factory_i *m_factory;

    std::string m_trigger;
};

#endif

