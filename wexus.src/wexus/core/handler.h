
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
 
#ifndef __INCLUDED_WEXUS_CORE_HANDLER_H__
#define __INCLUDED_WEXUS_CORE_HANDLER_H__

#include <scopira/tool/object.h>

namespace wexus
{
  namespace core
  {
    class handler_event;

    // base handler forward decls
    class handler_i;
    template<typename T1>
    class handler_imp_1;
    template<typename T1, typename T2>
    class handler_imp_2;
    template<typename T1, typename T2, typename T3>
    class handler_imp_3;
    template<typename T1, typename T2, typename T3, typename T4>
    class handler_imp_4;

    // function handler forward decls
    class function_handler_0;
    // 1 and on probably need explicit handler_i decendance
    template<typename T1>
    class function_handler_1;
    template<typename T1, typename T2>
    class function_handler_2;
    template<typename T1, typename T2, typename T3>
    class function_handler_3;
    template<typename T1, typename T2, typename T3, typename T4>
    class function_handler_4;

    // method handler forward decls
    template<typename C1>
    class method_handler_imp;
    template<typename C1>
    class method_handler_0;
    template<typename C1, typename T1>
    class method_handler_1;
    template<typename C1, typename T1, typename T2>
    class method_handler_2;
    template<typename C1, typename T1, typename T2, typename T3>
    class method_handler_3;
    template<typename C1, typename T1, typename T2, typename T3, typename T4>
    class method_handler_4;
  }
}

/**
 * base handler interface
 *
 * @author Andrew Kaspick
 */
class wexus::core::handler_i : public scopira::tool::object
{
  public:
    virtual void handle_app_event(handler_event& evt) = 0;
};

/**
 * base handler interface
 * accepts 1 additional parameter
 *
 * @author Andrew Kaspick
 */
template<typename T1>
class wexus::core::handler_imp_1
{
  public:
    handler_imp_1(T1 t1)
      : m_t1(t1) {}

  protected:
    T1 m_t1;
};

/**
 * base handler interface
 * accepts 2 additional parameters
 *
 * @author Andrew Kaspick
 */
template<typename T1, typename T2>
class wexus::core::handler_imp_2
{
  public:
    handler_imp_2(T1 t1, T2 t2)
      : m_t1(t1), m_t2(t2) {}

  protected:
    T1 m_t1;
    T2 m_t2;
};

/**
 * base handler interface
 * accepts 3 additional parameters
 *
 * @author Andrew Kaspick
 */
template<typename T1, typename T2, typename T3>
class wexus::core::handler_imp_3
{
  public:
    handler_imp_3(T1 t1, T2 t2, T3 t3)
      : m_t1(t1), m_t2(t2), m_t3(t3) {}

  protected:
    T1 m_t1;
    T2 m_t2;
    T3 m_t3;
};

/**
 * base handler interface
 * accepts 4 additional parameters
 *
 * @author Andrew Kaspick
 */
template<typename T1, typename T2, typename T3, typename T4>
class wexus::core::handler_imp_4
{
  public:
    handler_imp_4(T1 t1, T2 t2, T3 t3, T4 t4)
      : m_t1(t1), m_t2(t2), m_t3(t3), m_t4(t4) {}

  protected:
    T1 m_t1;
    T2 m_t2;
    T3 m_t3;
    T4 m_t4;
};

/**
 * function handler class
 * accepts 0 additional parameters
 *
 * @author Andrew Kaspick
 */
class wexus::core::function_handler_0 : public wexus::core::handler_i
{
  typedef void(*handler)(handler_event&);
  public:
    function_handler_0(handler h)
      : m_h(h) {}

    virtual void handle_app_event(handler_event& event)
      { m_h(event); }

  private:
    handler m_h;
};

/**
 * function handler class
 * accepts 1 additional parameter
 *
 * @author Andrew Kaspick
 */
template<typename T1>
class wexus::core::function_handler_1 : public wexus::core::handler_imp_1<T1>
{
  typedef void(*handler)(handler_event&, T1);
  public:
    function_handler_1(handler h, T1 t1)
      : handler_imp_1<T1>(t1), m_h(h) {}

    virtual void handle_app_event(handler_event& event)
      { handler_imp_1<T1>::m_h(event, handler_imp_1<T1>::m_t1); }

  private:
    /// handler function
    handler m_h;
};

/**
 * function handler class
 * accepts 2 additional parameters
 *
 * @author Andrew Kaspick
 */
template<typename T1, typename T2>
class wexus::core::function_handler_2 : public wexus::core::handler_imp_2<T1,T2>
{
  typedef void(*handler)(handler_event&, T1, T2);
  public:
    function_handler_2(handler h, T1 t1, T2 t2)
      : handler_imp_2<T1,T2>(t1, t2), m_h(h) {}

    virtual void handle_app_event(handler_event& event)
      { handler_imp_2<T1,T2>::m_h(event, handler_imp_2<T1,T2>::m_t1, handler_imp_2<T1,T2>::m_t2); }

  private:
    /// handler function
    handler m_h;
};

/**
 * function handler class
 * accepts 3 additional parameters
 *
 * @author Andrew Kaspick
 */
template<typename T1, typename T2, typename T3>
class wexus::core::function_handler_3 : public wexus::core::handler_imp_3<T1,T2,T3>
{
  typedef void(*handler)(handler_event&, T1, T2, T3);
  public:
    function_handler_3(handler h, T1 t1, T2 t2, T3 t3)
      : handler_imp_3<T1,T2,T3>(t1, t2, t3), m_h(h) {}

    virtual void handle_app_event(handler_event& event)
      { handler_imp_3<T1,T2,T3>::m_h(event, handler_imp_3<T1,T2,T3>::m_t1, handler_imp_3<T1,T2,T3>::m_t2, handler_imp_3<T1,T2,T3>::m_t3); }

  private:
    /// handler function
    handler m_h;
};

/**
 * function handler class
 * accepts 4 additional parameters
 *
 * @author Andrew Kaspick
 */
template<typename T1, typename T2, typename T3, typename T4>
class wexus::core::function_handler_4 : public wexus::core::handler_imp_4<T1,T2,T3,T4>
{
  typedef void(*handler)(handler_event&, T1, T2, T3, T4);
  public:
    function_handler_4(handler h, T1 t1, T2 t2, T3 t3, T4 t4)
      : handler_imp_4<T1,T2,T3,T4>(t1, t2, t3, t4), m_h(h) {}

    virtual void handle_app_event(handler_event& event)
      { handler_imp_4<T1,T2,T3,T4>::m_h(event, handler_imp_4<T1,T2,T3,T4>::m_t1, handler_imp_4<T1,T2,T3,T4>::m_t2, handler_imp_4<T1,T2,T3,T4>::m_t3, handler_imp_4<T1,T2,T3,T4>::m_t4); }

  private:
    /// handler function
    handler m_h;
};

/**
 * method handler implementation class
 * 
 * @param C1 class type of 'this' object
 * @author Andrew Kaspick
 */
template<typename C1>
class wexus::core::method_handler_imp : public wexus::core::handler_i
{
  public:
    /**
     * ctor
     *
     * @param 'this' object used to call method
     * @author Andrew Kaspick
     */
    method_handler_imp(C1* obj) : m_obj(obj) {}

  protected:
    C1* get_object() const { return m_obj; }

  private:
    C1* m_obj;
};

/**
 * method handler class
 * accepts 0 additional parameters
 *
 * @author Andrew Kaspick
 */
template<typename C1>
class wexus::core::method_handler_0 : public wexus::core::method_handler_imp<C1>
{
  typedef void (C1::*handler)(handler_event&);
  public:
    method_handler_0(C1* obj, handler h)
      : method_handler_imp<C1>(obj), m_h(h) {}

    virtual void handle_app_event(handler_event& event)
    {
      ((method_handler_imp<C1>::get_object())->*(m_h))(event);
    }

  private:
    /// handler method
    handler m_h;
};

/**
 * method handler class
 * accepts 1 additional parameter
 *
 * @author Andrew Kaspick
 */
template<typename C1, typename T1>
class wexus::core::method_handler_1
  : public wexus::core::method_handler_imp<C1>, public wexus::core::handler_imp_1<T1>
{
  typedef void (C1::*handler)(handler_event&, T1);
  public:
    method_handler_1(C1* obj, handler h, T1 t1)
      : method_handler_imp<C1>(obj), handler_imp_1<T1>(t1), m_h(h) {}

    virtual void handle_app_event(handler_event& event)
    {
      ((method_handler_imp<C1>::get_object())->*(m_h))(event, handler_imp_1<T1>::m_t1);
    }

  private:
    /// handler method
    handler m_h;
};

/**
 * method handler class
 * accepts 2 additional parameters
 *
 * @author Andrew Kaspick
 */
template<typename C1, typename T1, typename T2>
class wexus::core::method_handler_2
  : public wexus::core::method_handler_imp<C1>, public wexus::core::handler_imp_2<T1,T2>
{
  typedef void (C1::*handler)(handler_event&, T1, T2);
  public:
    method_handler_2(C1* obj, handler h, T1 t1, T2 t2)
      : method_handler_imp<C1>(obj), handler_imp_2<T1,T2>(t1, t2), m_h(h) {}

    virtual void handle_app_event(handler_event& event)
    {
      ((method_handler_imp<C1>::get_object())->*(m_h))(event, handler_imp_2<T1,T2>::m_t1, handler_imp_2<T1,T2>::m_t2);
    }

  private:
    /// handler method
    handler m_h;
};

/**
 * method handler class
 * accepts 3 additional parameters
 *
 * @author Andrew Kaspick
 */
template<typename C1, typename T1, typename T2, typename T3>
class wexus::core::method_handler_3
  : public wexus::core::method_handler_imp<C1>, public wexus::core::handler_imp_3<T1,T2,T3>
{
  typedef void (C1::*handler)(handler_event&, T1, T2, T3);
  public:
    method_handler_3(C1* obj, handler h, T1 t1, T2 t2, T3 t3)
      : method_handler_imp<C1>(obj), handler_imp_3<T1,T2,T3>(t1, t2, t3), m_h(h) {}

    virtual void handle_app_event(handler_event& event)
    {
      ((method_handler_imp<C1>::get_object())->*(m_h))(event, handler_imp_3<T1,T2,T3>::m_t1, handler_imp_3<T1,T2,T3>::m_t2, handler_imp_3<T1,T2,T3>::m_t3);
    }

  private:
    /// handler method
    handler m_h;
};

/**
 * method handler class
 * accepts 4 additional parameters
 *
 * @author Andrew Kaspick
 */
template<typename C1, typename T1, typename T2, typename T3, typename T4>
class wexus::core::method_handler_4
  : public wexus::core::method_handler_imp<C1>, public wexus::core::handler_imp_4<T1,T2,T3,T4>
{
  typedef void (C1::*handler)(handler_event&, T1, T2, T3, T4);
  public:
    method_handler_4(C1* obj, handler h, T1 t1, T2 t2, T3 t3, T4 t4)
      : method_handler_imp<C1>(obj), handler_imp_4<T1,T2,T3,T4>(t1, t2, t3, t4), m_h(h) {}

    virtual void handle_app_event(handler_event& event)
    {
      ((method_handler_imp<C1>::get_object())->*(m_h))(event);
    }

  private:
    /// handler method
    handler m_h;
};

#endif

