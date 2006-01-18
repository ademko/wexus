
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

#include <wexus/ui/form.h>

#include <scopira/tool/flow.h>

//BBtargets libwexus.so

using namespace wexus::ui;

//
//
// submit_event
//
//

submit_event::submit_event(form_event *fe, form *fm, error_list *el)
  : m_subevent(fe), m_form(fm), m_errors(el)
{
  assert(fe);
  assert(fm);
}

void submit_event::add_error(const std::string &err) const
{
  if (m_errors)
    m_errors->push_back(err);
}

//
//
// page
//
//

page::page(void)
  : m_title("Untitled")
{
}

void page::render(scopira::tool::oflow_i &out)
{
  out << "<HTML><HEAD><TITLE>" << m_title << "</TITLE></HEAD><BODY>\n";
  parent_type::render(out);
  out << "</BODY></HTML>\n";
}

//
//
// form
//
//

form::form(void)
{
}

void form::render(scopira::tool::oflow_i &out)
{
  out << "<FORM>\n"
         "<INPUT TYPE=\"HIDDEN\" NAME=\"c\" VALUE=\"submit\"/>\n"
         "<INPUT TYPE=\"HIDDEN\" NAME=\"t\" VALUE=\"" << get_tag()
         << "\"/>\n";
  parent_type::render(out);
  out << "<INPUT TYPE=\"SUBMIT\" VALUE=\"SUBMIT\"/>\n"
         "</FORM>\n";
}

/*void form::handle(form_event &ev)
{
  // submit stuff used to be here
}*/

bool form::dispatch(form_event &ev)
{
  if (ev.get_tag() == get_tag() && ev.get_command() == "submit") {
    submit_event subev(&ev, this, m_errors.get());

    if (m_errors.get())
      m_errors->clear();
    // do form submission
    if (handle_submit(subev) == 0)
      on_submit(subev);  // no errors? call user handler
    return true;
  } else
    return parent_type::dispatch(ev);
}

void form::on_submit(submit_event &ev)
{
  // nothing
}

void form::add_error_list(error_list *el)
{
  assert(m_errors.is_null());

  m_errors = el;

  add_element(el);
}

