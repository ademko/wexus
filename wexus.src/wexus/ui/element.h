
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

#ifndef __INCLUDED_WEXUS_UI_ELEMENT_HPP__
#define __INCLUDED_WEXUS_UI_ELEMENT_HPP__

#include <string>
#include <list>

#include <scopira/tool/object.h>
#include <scopira/tool/array.h>
#include <wexus/core/app.h>
#include <wexus/db/connection.h>
#include <wexus/db/pool.h>

namespace scopira
{
  namespace tool
  {
    class oflow_i;
  }
}

namespace wexus
{
  namespace ui
  {
    class form_event;
    class submit_event;   //forward

    class element;
    class form_element;

    class element_list;
    class element_matrix;

    class error_list;

    class form_session;   //forward
  }
}

/**
 * A form event.
 * @author Aleksander Demko
 */
class wexus::ui::form_event
{
  private:
    int m_tag;
    const std::string *m_command;
    core::app_event *m_app_event;

    enum op_t {
      op_none_c,
      op_replace_c,
      op_push_c,
      op_pop_c,
    };

    op_t m_op;

    scopira::tool::count_ptr<element> m_replace_old, m_replace_new;

    db::connection_pool *m_conn_pool;     /// might be null
    scopira::tool::count_ptr< wexus::db::connection > m_cur_conn;

  public:
    /// ctor
    form_event(int tag, const std::string *cmd, core::app_event *appev, db::connection_pool *dbpool);
    /// dtor
    ~form_event();

    /// gets the target tag
    int get_tag(void) const { return m_tag; }
    /// gets the command
    const std::string & get_command(void) const { return *m_command; }
    /// get a form field, if any
    const std::string & get_form_field(const std::string &fieldname) const { return m_app_event->get_form_field(fieldname); }

    /**
     * Queues a replace. The pointers will be count_ptr'ed.
     * Forms and pages often call this to "replace" themselves
     * as users "advance" through the application.
     * @author Aleksander Demko
     */
    void element_replace(element *olde, element *newe);

    /**
     * Similar to element_replace(), but "push" the current
     * version onto the stack, for later retrieval with pop
     * @author Aleksander Demko
     */
    void element_push(element *olde, element *newe);

    /**
     * Restored a previously pushed element.
     * @author Aleksander Demko
     */ 
    void element_pop(element *olde);

    /**
     * Get a db connection from the pool.
     * @author Aleksander Demko
     */ 
    db::connection & db(void);

    friend class wexus::ui::form_session;
};

/**
 * The basis of all form widgets (elements)
 * @author Aleksander Demko
 */
class wexus::ui::element : public virtual scopira::tool::object
{
  private:
    element_list *m_parent;
  public:
    /**
     * Get my "id" (unique tag)
     * @author Aleksander Demko
     */
    int get_tag(void) const { return reinterpret_cast<int>(this); }

    /**
     * Gets the parent form
     * @author Aleksander Demko
     */
    element_list * get_parent(void) const { return m_parent; }

    /**
     * Gets the parent that has no parent itself (recursivly).
     *
     * @author Aleksander Demko
     */
    element_list * get_top_parent(void);

    /**
     * Sets the parent form
     * @author Aleksander Demko
     */
    void set_parent(element_list *p);

    /**
     * Render this element to the given output stream.
     *
     * Future enhancements could include a structural render
     * system to allow post production fine tuning at the
     * html level.
     *
     * This one does nothing.
     *
     * @author Aleksander Demko
     */ 
    virtual void render(scopira::tool::oflow_i &out);
    /**
     * Handles a form event.
     * Default does nothing.
     * @author Aleksander Demko
     */
    virtual void handle(form_event &ev);
    /**
     * Called when a formevent went to the form.
     * This is called when the form is (attempeted) to be commited.
     * Returns the number of errors.
     * @author Aleksander Demko
     */
    virtual int handle_submit(submit_event &ev);
    /**
     * Routes events to handle() methods.
     * Returns true if the ONLY target was found.
     * This routine is mostly internal.
     * @author Aleksander Demko
     */
    virtual bool dispatch(form_event &ev);
    /**
     * Propagate a reparent call.
     * This implementation does nothing.
     * Returns true on found.
     * This routine is mostly internal.
     * @author Aleksander Demko
     */ 
    virtual bool reparent(element *olde, element *newe);

  protected:
    /// ctor
    element(void);
};

/**
 * For elements that belong in a form. Adds facilities for naming
 * and data validation.
 * @author Aleksander Demko
 */ 
class wexus::ui::form_element : public wexus::ui::element
{
  public:
    /**
     * Gets the name of this widget, if any.
     * @author Aleksander Demko
     */
    const std::string &get_name(void) const { return m_name; }

  protected:
    /// ctor
    form_element(const std::string &_name);
  protected:
    std::string m_name;
};

/**
 * A form (collection of elements)
 * @author Aleksander Demko
 */
class wexus::ui::element_list : public wexus::ui::element
{
  private:
    typedef element parent_type;

    typedef std::list<scopira::tool::count_ptr<element> > element_set;

    /// the elements
    element_set m_elements;

  protected:
    /// simple ctor
    element_list(void);

    /// inherited (broadcaster)
    virtual void render(scopira::tool::oflow_i &out);
    /// inherited (broadcaster)
    //virtual void handle(form_event &ev);
    /// inherited (broadcaster)
    virtual int handle_submit(submit_event &ev);
    /// inherited (broadcaster)
    virtual bool dispatch(form_event &ev);
    /// inherited (broadcaster)
    virtual bool reparent(element *olde, element *newe);

    /**
     * Add an element with to the given element set.
     * It will be count_ptr'ed.
     * el is returned.
     * @author Aleksander Demko
     */
    element * add_element(element *el);
};

/**
 * A matrix of elements. A base class.
 * @author Aleksander Demko
 */
class wexus::ui::element_matrix : public wexus::ui::element
{
  protected:
    typedef element parent_type;

    struct cell_t {
      scopira::tool::count_ptr<element> el;
      size_t w, h;
      bool active;

      cell_t(void) : active(false) { }
    };
    typedef scopira::tool::basic_matrix<cell_t> element_set;

    element_set m_elements;

  public:
    /// inherited (broadcaster)
    virtual void render(scopira::tool::oflow_i &out);
    /// inherited (broadcaster)
    //virtual void handle(form_event &ev);
    /// inherited (broadcaster)
    virtual int handle_submit(submit_event &ev);
    /// inherited (broadcaster)
    virtual bool dispatch(form_event &ev);
    /// inherited (broadcaster)
    virtual bool reparent(element *olde, element *newe);

    /**
     * Resize the table.
     * @author Aleksander Demko
     */
    void resize(size_t w, size_t h) { m_elements.resize(w,h); }
    /**
     * Returns the width of the matrix.
     * @author Aleksander Demko
     */ 
    size_t width(void) const { return m_elements.width(); }
    /**
     * Returns the height of the matrix.
     * @author Aleksander Demko
     */ 
    size_t height(void) const { return m_elements.height(); }
    /**
     * Add an element with to the given element set.
     * It will be count_ptr'ed.
     * el is returned.
     * @author Aleksander Demko
     */
    element * add_element(size_t x, size_t y, size_t spanw, size_t spanh, element *el);

  protected:
    /// ctor
    element_matrix(void);
};

/**
 * A list of errors.
 * @author Aleksander Demko
 */
class wexus::ui::error_list : public wexus::ui::element
{
  private:
    typedef std::list<std::string> list_type;
    list_type m_errors;
  public:
    /// ctor
    error_list(void);
    /// inherited
    virtual void render(scopira::tool::oflow_i &out);

    /// is it empty?
    bool empty(void) const { return m_errors.empty(); }
    /// clear (empty) the error list
    void clear(void) { m_errors.clear(); }
    /// adds an error
    void push_back(const std::string &err);
};

#endif

