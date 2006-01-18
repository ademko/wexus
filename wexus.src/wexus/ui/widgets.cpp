

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

#include <wexus/ui/widgets.h>

#include <scopira/tool/flow.h>
#include <wexus/ui/form.h>

//BBtargets libwexus.so

using namespace wexus::ui;

//
//
// label
//
//

label::label(const std::string &lab)
  : m_label(lab)
{
}

void label::render(scopira::tool::oflow_i &out)
{
  out << m_label;
}

//
//
// cmd_link
//
//

cmd_link::cmd_link(element *target, const std::string &cmd, const std::string &desc)
  : m_command(cmd), m_desc(desc)
{
  if (target)
    m_target = target->get_tag();
  else
    m_target = 0;
}

void cmd_link::render(scopira::tool::oflow_i &out)
{
  out << "<A HREF=\"form?c=" << m_command << "&t=" << m_target
    << "\">" << m_desc << "</a>";
}

//
//
// pop_link
//
//

pop_link::pop_link(element *target, const std::string &desc)
  : m_desc(desc), m_target(target)
{
  assert(target);
}

void pop_link::render(scopira::tool::oflow_i &out)
{
  out << "<A HREF=\"form?c=x&t=" << get_tag()
    << "\">" << m_desc << "</a>";
}

void pop_link::handle(form_event &ev)
{
  ev.element_pop(m_target);
}

//
//
// textfield
//
//

textfield::textfield(const std::string &name, const std::string &def,
      int sz, int maxlen, int type)
  : parent_type(name), m_val(def), m_size(sz), m_maxlen(maxlen), m_type(type)
{
}

void textfield::render(scopira::tool::oflow_i &out)
{
  out << "<INPUT TYPE=\""
    << (m_type == password_c ? "password" : "text")
    << "\" NAME=\"" << m_name << "\" VALUE=\""
    << m_val << "\" SIZE=\"" << m_size << "\" MAXLENGTH=\""
    << m_maxlen << "\" />\n";
}

int textfield::handle_submit(submit_event &ev)
{
  m_val = ev.get_form_field(m_name);
  return 0;
}

//
//
// table
//
//

table::table(void)
{
}

void table::render(scopira::tool::oflow_i &out)
{
  size_t x, y;

  out << "<TABLE>\n";

  for (y=0; y<m_elements.height(); ++y) {
    out << "<TR>\n";
    for (x=0; x<m_elements.width(); ++x) {
      cell_t &c = m_elements(x,y);
      
      if (c.active) {
        if (c.el.get()) {
          out << "<TD";
          if (c.w>1)
            out << " COLSPAN=" << c.w;
          if (c.h>1)
            out << " ROWSPAN=" << c.h;
          out << '>';
          c.el->render(out);
          out << "</TD>";
        }
      } else
        out << "<TD></TD>";
    }
    out << "</TR>\n";
  }

  out << "</TABLE>\n";
}

