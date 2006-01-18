
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

#ifndef __INCLUDED_WEXUS_UI_FORM_HPP__
#define __INCLUDED_WEXUS_UI_FORM_HPP__

#include <wexus/ui/element.h>

namespace wexus
{
  /**
   * This namespace contains the various base classes and simple
   * elements for the form state managment system.
   * @author Aleksander Demko
   */ 
  namespace ui
  {
    class submit_event;
    class page;
    class form;
  }
}

/**
 * A particular form_event that also has form submittion information.
 * @author Aleksander Demko
 */
class wexus::ui::submit_event
{
  private:
    form_event *m_subevent;
    form *m_form;
    error_list *m_errors;

  public:
    /// el may be null
    submit_event(form_event *fe, form *fm, error_list *el);

    /// gets the target tag
    int get_tag(void) const { return m_subevent->get_tag(); }
    /// gets the command
    const std::string & get_command(void) const { return m_subevent->get_command(); }
    /// get a form field, if any
    const std::string & get_form_field(const std::string &fieldname) const { return m_subevent->get_form_field(fieldname); }

    /// as in form_event
    void element_replace(element *olde, element *newe) { m_subevent->element_replace(olde, newe); }
    /// as in form_event
    void element_push(element *olde, element *newe) { m_subevent->element_push(olde, newe); }
    /// as in form_event
    void element_pop(element *olde) { m_subevent->element_pop(olde); }

    /// as in form_event
    db::connection & db(void) { return m_subevent->db(); }

    /// gets the submitting form
    form * get_form(void) const { return m_form; }
    /// adds an error
    void add_error(const std::string &err) const;
};

/**
 * A simple (complete) web page.
 * @author Aleksander Demko
 */
class wexus::ui::page : public wexus::ui::element_list
{
  private:
    typedef element_list parent_type;

    std::string m_title;    /// page name

  public:
    /// ctor
    page(void);
    /// inherited
    virtual void render(scopira::tool::oflow_i &out);
};

/**
 * A simple (complete) web page.
 * @author Aleksander Demko
 */
class wexus::ui::form : public wexus::ui::element_list
{
  private:
    typedef element_list parent_type;

    scopira::tool::count_ptr<error_list> m_errors;

  public:
    /// ctor
    form(void);
    /// inherited
    virtual void render(scopira::tool::oflow_i &out);
    /// inherited
    //virtual void handle(form_event &ev);

    /// inherited
    virtual bool dispatch(form_event &ev);
    /**
     * Called when the form is *successfully* submitted.
     * Descendants should override this and do something.
     *
     * This implementation does nothing.
     *
     * @author Aleksander Demko
     */
    virtual void on_submit(submit_event &ev);

    /**
     * Sets the current error_list attached to this form.
     * Null will unset.
     * Will be count_ptr'ed.
     * @author Aleksander Demko
     */
    void add_error_list(error_list *el);
};

#endif

