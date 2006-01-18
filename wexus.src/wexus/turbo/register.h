
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

#ifndef __INCLUDED_WEXUS_TURBO_REGISTER_H__
#define __INCLUDED_WEXUS_TURBO_REGISTER_H__

#include <string>
#include <map>

#include <scopira/tool/object.h>
#include <wexus/turbo/turboreactor.h>
#include <wexus/turbo/widgets.h>
#include <wexus/turbo/exception.h>

namespace wexus
{
  namespace turbo
  {
    class turbo_event; //fwd

    class registry;
    class register_function;
      
    template <class C, void (C::*METH)(void)>
      class register_controller;
  }
}

/**
 * a singleton registry of turbo functions
 *
 * @author Aleksander Demko
 */
class wexus::turbo::registry
{
  public:
    /// a turbo funcion (wtc)
    typedef void (*tfunc_t)(wexus::turbo::turbo_event&);

  public:
    static registry * instance(void);

    /// ctor
    virtual ~registry();

    /// custom add_ref like method
    int add_ref(void) const;
    /// custom sub_ref like method
    bool sub_ref(void) const;

    /// alternate just for performance reasons
    void add_type(const std::string & name, tfunc_t loader);
    /// alternate just for performance reasons
    void add_type(const char * name, tfunc_t loader);
    /// specialized version for the controll infrastructure
    void add_controller_type(const char *fullclassname, const char *methodname, tfunc_t loader);
    /// returns the previously registered url for a given fullclassname and method
    /// assert-FAILS if its not found
    std::string get_controller_function(const std::string &fullclassname, const std::string &methodname);

    /// gets a type, may return null
    tfunc_t get_type(const std::string &name) const;

  private:
    /// ctor
    registry(void);

  private:
    mutable int dm_refcount;
    typedef std::map<std::string, tfunc_t> funcstore_t;

    funcstore_t m_store;
};

/**
 * Useful registration function... auto created by wxmake
 *
 * @author Aleksander Demko
 */ 
class wexus::turbo::register_function
{
  public:
    /// ctor
    register_function(const char * funcname, wexus::turbo::registry::tfunc_t func)
      { wexus::turbo::registry::instance()->add_type(funcname, func); }
};

/**
 * The registration object used to register methods of controllers.
 * @author Aleksander Demko
 */
template <class C, void (C::*METH)(void)> class wexus::turbo::register_controller
{
  private:
    template <class LC, void (C::*LMETH)(void)>
      static void func_to_method(wexus::turbo::turbo_event &ev)
      {
        // instantiate the object (this ofcourse calls the controllers ctor)
        try {
          LC c;
          // did the ctor set any errors?
          if (wexus::turbo::get_tls_event().has_error())
            wexus::turbo::render_errors();  // set, call the error handler
          else
            (c.*LMETH)(); // no? then call the designated method
        }
        //catch (const wexus::turbo::redirect_to_error &e) {
        //}
        catch (const wexus::turbo::turbo_error &e) {
          //if (!wexus::turbo::get_tls_event().has_error())
          // an empty what means i shouldnt add the error
          if (e.what() && e.what()[0])
            wexus::turbo::get_tls_event().add_error("exception", e.what());
          if (wexus::turbo::get_tls_event().has_error())
            wexus::turbo::render_errors();  // set, call the error handler
        }
      }
  public:
    /// ctor
    register_controller(const char *fullclassname, const char *methodname)
      { wexus::turbo::registry::instance()->add_controller_type(fullclassname, methodname,
          func_to_method<C, METH>); }
};

#endif

