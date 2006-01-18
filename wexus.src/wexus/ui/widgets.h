
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

#ifndef __INCLUDED_WEXUS_UI_WIDGETS_HPP__
#define __INCLUDED_WEXUS_UI_WIDGETS_HPP__

#include <wexus/ui/element.h>

namespace wexus
{
  namespace ui
  {
    class label;
    class cmd_link;
    class pop_link;
    class textfield;

    class table;
  }
}

/**
 * A label.
 * @author Aleksander Demko
 */
class wexus::ui::label : public wexus::ui::element
{
  private:
    std::string m_label;
  public:
    /// ctor
    label(const std::string &lab);
    /// inherited
    virtual void render(scopira::tool::oflow_i &out);
};

/**
 * A link.
 * @author Aleksander Demko
 */
class wexus::ui::cmd_link : public wexus::ui::element
{
  private:
    std::string m_command, m_desc;
    int m_target;
  public:
    /// ctor, target may be null
    cmd_link(element *target, const std::string &cmd, const std::string &desc);
    /// inherited
    virtual void render(scopira::tool::oflow_i &out);
};

/**
 * A link that calls pop on the given element.
 * @author Aleksander Demko
 */
class wexus::ui::pop_link : public wexus::ui::element
{
  private:
    element * m_formtocall;
  public:
  private:
    std::string m_desc;
    element *m_target;
  public:
    /**
     * Ctor. target is the form that will be passed
     * to the element_pop call. It will NOT be count_ptr'ed.
     * @author Aleksander Demko
     */ 
    pop_link(element *target, const std::string &desc = "Cancel");
    /// inherited
    virtual void render(scopira::tool::oflow_i &out);
    /// inherited
    virtual void handle(form_event &ev);
};

/**
 * A single line text field.
 * @author Aleksander Demko
 */
class wexus::ui::textfield : public wexus::ui::form_element
{
  public:
    enum {
      normal_c = 0,
      password_c = 1
    };

  private:
    typedef form_element parent_type;

    std::string m_val;        // default string
    int m_size;          // size
    int m_maxlen;        // maxlen
    int m_type;          // the button type

  public:
    /// ctor
    textfield(const std::string &name, const std::string &def = "",
      int sz = 30, int maxlen = 60, int type = normal_c);

    /// inherited
    virtual void render(scopira::tool::oflow_i &out);
    /// inherited
    virtual int handle_submit(submit_event &ev);

    /// Does this widget contain good data?
    //virtual bool check_error(std::string &error) const { return false; }

    /// gets the current value
    const std::string &get_value(void) const { return m_val; }
};

/**
 * An html table/grid
 * @author Aleksander Demko
 */ 
class wexus::ui::table : public wexus::ui::element_matrix
{
  public:
    /// ctor
    table(void);
    /// inherited
    virtual void render(scopira::tool::oflow_i &out);
};

#endif

