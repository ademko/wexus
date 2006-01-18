
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

#ifndef __INCLUDED_WEXUS_CORE_ENGINE_HPP__
#define __INCLUDED_WEXUS_CORE_ENGINE_HPP__

#include <vector>

#include <scopira/tool/thread.h>
#include <scopira/tool/dll.h>
#include <scopira/tool/netflow.h>

#include <wexus/core/front.h>
#include <wexus/core/app.h>
#include <wexus/core/mimetypemanager.h>
#include <wexus/core/checkers.h>

namespace scopira
{
  namespace tool
  {
    // fwd
    class iflow_i;
  }
}

namespace wexus
{
  /**
   * core contains the Wexus engine core and basic application facilities.
   *
   * @author Aleksander Demko
   */ 
  namespace core
  {
    class engine;
  }
}

/**
 * the main engine
 *
 * @author Aleksander Demko
 */
class wexus::core::engine : public virtual scopira::tool::job_i,
  public virtual wexus::core::front_peer_i
{
  public:
    /// constructor
    engine(void);
    /// destructor
    virtual ~engine();

    static inline engine * instance(void) { return dm_instance; }

    // front_peer_i

    /// the front should call this on event request
    virtual void handle_front_event(front_event_i& evt);

    /// this signals the peer to begin total shutdown
    virtual void handle_shutdown(void);

    /**
     * config file loader. reads a config file from the given filename,
     * into the given recprop structure. loads any needed fronts.
     * returns true on success
     *
     * @author Aleksander Demko
     */ 
    bool load_prop_file(const std::string &filename, scopira::tool::property *out);

    /**
     * the actual guts of the previous function
     *
     * @author Aleksander Demko
     */
    bool load_prop_file(scopira::tool::iflow_i &in, scopira::tool::property *out);

    /**
     * adds a front to be managed by the engine.
     * the engine will not start_front it though
     *
     * @param front the front to add, it will be ref counted
     * @author Aleksander Demko
     */
    void add_front(front_i *front);

    /**
     * adds an app, at the given mount point, to be managed
     * by the engine. mount points look like "/blah/".
     * "/" is the default mount point.
     *
     * @author Aleksander Demko
     */
    void add_app(const std::string &mntpoint, app *a);

    /// starts the engine
    void start(void);

    /// notifys the engine that it should begin shutdown
    void notify_stop(void);

    /// blocks until the engine is told to stop
    void wait_stop(void);

    /// return if the job is running
    virtual bool is_running(void) const;

    /// return the current mimetype manager. yes, const.
    const mimetype_manager &get_mimetype_manager(void) const
      { return m_mimeman; }

    checkers & get_checkers(void)
      { return m_checkers; }

  private:
    // using vector instead of list, cuz they iterate faster
    // (and we dont need any post initialization editing anyways)

    static engine * dm_instance;

    typedef std::vector< scopira::tool::count_ptr<scopira::tool::dll> > dlllist_t;
    typedef std::vector< scopira::tool::count_ptr< front_i > > frontlist_t;
    typedef std::pair< std::string, scopira::tool::count_ptr< app > > appentry_t;
    typedef std::vector< appentry_t > applist_t;

    scopira::tool::event_area<bool> m_shutdown;
    dlllist_t m_dlls;                               /// loaded libs
    frontlist_t m_fronts;                           /// fronts
    applist_t m_apps;                                /// app and their mount points

    mimetype_manager m_mimeman;                     /// the mime type manager
    checkers m_checkers;                            /// the checkers instance

    scopira::tool::net_loop m_aloop;

  private:
    /// loads an object, null on not found
    scopira::tool::object *load_object(const std::string &name);
    /// loads a lib
    void load_lib(const std::string &s);
    /// loads a front from the prop file
    void load_front(scopira::tool::property *rp);
    /// loads an app from the prop file
    void load_app(scopira::tool::property *rp);
};

#endif

