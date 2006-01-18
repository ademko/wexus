
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

#ifndef __INCLUDED_WEXUS_CORE_APP_HPP__
#define __INCLUDED_WEXUS_CORE_APP_HPP__

#include <string>
#include <vector>

#include <scopira/tool/thread.h>

#include <wexus/core/reactor.h>
#include <wexus/core/front.h>

namespace scopira
{
  namespace tool
  {
    class oflow_i;  //forward
    class property;
  }
}

namespace wexus
{
  namespace core
  {
    class app_event_data;
    class app_event;
    class app;
  }
}

/**
 * an application event. the engine sends these to the app
 * and the app propagates these to the reactors
 *
 * @author Aleksander Demko
 */
class wexus::core::app_event_data
{
  public:
    /// simple ctor
    app_event_data(const std::string &oreq);

    /// gets the orignal request
    const std::string & get_orig_req(void) const { return m_origreq; }

  private:
    const std::string &m_origreq;
};

/**
 * a pair of ptrs to a front and app_event
 *
 * @author Aleksander Demko
 */
class wexus::core::app_event
{
  public:
    // public members, all refs
    const std::string & req;
    front_event_i & front;
    app_event_data & app;

    /// initing ctor - will maintain ptr to the given string!
    app_event(const std::string &reqe, front_event_i &fronte, app_event_data &appe)
      : req(reqe), front(fronte), app(appe) { }

    // default copy ctor and dtor are fine.

    // safe routines

    /// gets a form field from the front, always works
    const std::string & get_form_field(const std::string &fieldname) const;
    /// checks if the form field exists
    bool has_form_field(const std::string& name) const { return front.has_form_field(name); }
};

/**
 * the base for all "applications". an engine can have many
 * applications. 3rd party developers extend this class and
 * add their applications to the engine
 *
 * @author Aleksander Demko
 */
class wexus::core::app : public virtual scopira::tool::job_i
{
  public:
    /// simple ctor
    app(void);
    /// simple dtor
    virtual ~app();

    /**
     * Called when you are to load parameters from the type tree.
     * Returns true on success.
     *
     * You may want to implement this.
     * Default implementation does nothing.
     *
     * @author Aleksander Demko
     */ 
    virtual bool init_prop(const std::string &fist_mntpt, scopira::tool::property *prop);

    // job stuff - provide simple, default implementaions

    /// starts the job
    virtual void start(void);
    /// tells the job to try to stop
    virtual void notify_stop(void);
    /// this blocks until the job stops
    virtual void wait_stop(void);
    /// return if the job is running
    virtual bool is_running(void) const;

    /**
     * called when its time to handle an event
     *
     * this version goes through the reactor stack, looking for
     * a reactor to handle this event
     * @author Aleksander Demko
     */ 
    virtual void handle_app_event(app_event &evt);

  protected:

    /**
     * adds the given reactor to the local stack - this reactor
     * will be ref counted by the app
     *
     * @author Aleksander Demko
     */
    void add_reactor(reactor_i *rea);

  private:

    typedef std::vector< scopira::tool::count_ptr< reactor_i > > reactors_t;
    /// vector of reactors
    // its a vector for speedy iteration (plus we dont
    // need the post-manipulating facilities of list)
    reactors_t m_reactors;
};

#endif

