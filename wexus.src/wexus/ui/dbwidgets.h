
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

#ifndef __INCLUDED_WEXUS_UI_DBWIDGETS_HPP__
#define __INCLUDED_WEXUS_UI_DBWIDGETS_HPP__

#include <wexus/db/dbtypes.h>
#include <wexus/ui/element.h>

namespace wexus
{
  namespace ui
  {
    class dbstring_textfield;
  }
}

class wexus::ui::dbstring_textfield : public wexus::ui::form_element
{
  public:
    enum {
      normal_c = 0,
      password_c = 1
    };

  private:
    typedef form_element parent_type;

    db::dbstring m_val;        // default string
    int m_size;          // size
    int m_maxlen;        // maxlen
    int m_type;          // the button type

  public:
    /// ctor
    dbstring_textfield(const std::string &name,
      int sz = 30, int maxlen = 60, int type = normal_c);

    /// inherited
    virtual void render(scopira::tool::oflow_i &out);
    /// inherited
    virtual int handle_submit(submit_event &ev);

    /// Does this widget contain good data?
    //virtual bool check_error(std::string &error) const { return false; }

    /// gets the current value
    db::dbstring& ref(void) { return m_val; }
    /// gets the current value
    db::dbstring* ptr(void) { return &m_val; }
};

#endif

