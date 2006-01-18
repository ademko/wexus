
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

#include <wexus/ui/dbwidgets.h>

#include <wexus/ui/form.h>

//BBtargets libwexus.so

using namespace wexus::ui;

//
//
// dbstring_textfield
//
//

dbstring_textfield::dbstring_textfield(const std::string &name,
      int sz, int maxlen, int type)
  : parent_type(name), m_size(sz), m_maxlen(maxlen), m_type(type)
{
}

void dbstring_textfield::render(scopira::tool::oflow_i &out)
{
  out << "<INPUT TYPE=\""
    << (m_type == password_c ? "password" : "text")
    << "\" NAME=\"" << m_name << "\" VALUE=\""
    << m_val.c_str() << "\" SIZE=\"" << m_size << "\" MAXLENGTH=\""
    << m_maxlen << "\" />\n";
}

int dbstring_textfield::handle_submit(submit_event &ev)
{
  m_val.set( ev.get_form_field(m_name) );
  return 0;
}

