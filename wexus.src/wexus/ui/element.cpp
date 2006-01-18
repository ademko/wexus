
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

#include <wexus/ui/element.h>

#include <scopira/tool/flow.h>

//BBtargets libwexus.so

using namespace wexus;
using namespace wexus::db;
using namespace wexus::ui;

//
//
// form_event
//
//

form_event::form_event(int tag, const std::string *cmd, core::app_event *appev,
  db::connection_pool *dbpool)
  : m_tag(tag), m_command(cmd), m_app_event(appev), m_conn_pool(dbpool)
{
  assert(cmd);
  assert(appev);

  m_op = op_none_c;
}

form_event::~form_event()
{
  if (m_cur_conn.get()) {
    // return the connection
    m_conn_pool->push(m_cur_conn);  // check out one
    m_cur_conn = 0;
  }
}

void form_event::element_replace(element *olde, element *newe)
{
  m_op = op_replace_c;

  assert(olde);
  assert(newe);
  m_replace_old = olde;
  m_replace_new = newe;
}

void form_event::element_push(element *olde, element *newe)
{
  m_op = op_push_c;

  m_replace_old = olde;
  m_replace_new = newe;
}

void form_event::element_pop(element *olde)
{
  m_op = op_pop_c;

  m_replace_old = olde;
}

connection & form_event::db(void)
{
  if (m_cur_conn.get())
    return m_cur_conn.ref();    // return existing
  if (!m_conn_pool) {
    assert(false);
    return m_cur_conn.ref();    // just so the compile would shut up
  }

  m_conn_pool->pop(m_cur_conn);  // check out one
  return m_cur_conn.ref();
}

//
//
// element
//
//

element::element(void)
  : m_parent(0)
{
}

element_list * element::get_top_parent(void)
{
  element_list *t;

  if (!m_parent)
    return dynamic_cast<element_list*>(this);   // null is ok here, i guess

  t = get_parent();
  while (t->get_parent())
    t = t->get_parent();

  return t;
}

void element::set_parent(element_list *p)
{
  m_parent = p;
}

void element::render(scopira::tool::oflow_i &out)
{
  // nothin
}

void element::handle(form_event &ev)
{
  // nothin
}

int element::handle_submit(submit_event &ev)
{
  return 0;
}

bool element::dispatch(form_event &ev)
{
  if (get_tag() == 0) {
    handle(ev);   //broadcast
    return false;
  } if (ev.get_tag() == get_tag()) {
    handle(ev);   // found it, route here
    return true;
  } else
    return false;
}

bool element::reparent(element *olde, element *newe)
{
  // nothin
  return false;
}

//
//
// form_element
//
//

form_element::form_element(const std::string &_name)
  : m_name(_name)
{
}

//
//
// element_list
//
//

element_list::element_list(void)
{
}

void element_list::render(scopira::tool::oflow_i &out)
{
  element_set::iterator ii, endii;

  endii = m_elements.end();
  for (ii=m_elements.begin(); ii != endii; ++ii)
    (*ii)->render(out);
}

/*void element_list::handle(form_event &ev)
{
  element_set::iterator ii, endii;

  endii = m_elements.end();
  for (ii=m_elements.begin(); ii != endii; ++ii)
    (*ii)->handle(ev);
}
*/

int element_list::handle_submit(submit_event &ev)
{
  int ret;
  element_set::iterator ii, endii;

  ret = 0;
  endii = m_elements.end();
  for (ii=m_elements.begin(); ii != endii; ++ii)
    ret += (*ii)->handle_submit(ev);

  return ret;
}

bool element_list::dispatch(form_event &ev)
{
  element_set::iterator ii, endii;
  bool b;

  b = parent_type::dispatch(ev);    // self check

  endii = m_elements.end();
  for (ii=m_elements.begin(); !b && ii != endii; ++ii)
    b = (*ii)->dispatch(ev);

  return b;
}

bool element_list::reparent(element *olde, element *newe)
{
  element_set::iterator ii, endii;

  // first pass, check if I have it

  endii = m_elements.end();
  for (ii=m_elements.begin(); ii != endii; ++ii)
    if ((*ii) == olde) {
      // found it!
      (*ii) = newe;
      return true;
    }

  // second pass, recurse

  endii = m_elements.end();
  for (ii=m_elements.begin(); ii != endii; ++ii)
    if ( (*ii)->reparent(olde,newe) )
      return true;

  return false;
}

element * element_list::add_element(element *el)
{
  assert(el);

  m_elements.push_back(el);

  return el;
}

//
//
// element_matrix
//
//

element_matrix::element_matrix(void)
{
}

void element_matrix::render(scopira::tool::oflow_i &out)
{
  element_set::iterator ii, endii;

  endii = m_elements.end();
  for (ii=m_elements.begin(); ii != endii; ++ii)
    ii->el->render(out);
}

int element_matrix::handle_submit(submit_event &ev)
{
  int ret;
  element_set::iterator ii, endii;

  ret = 0;
  endii = m_elements.end();
  for (ii=m_elements.begin(); ii != endii; ++ii)
    ret += ii->el->handle_submit(ev);

  return ret;
}

bool element_matrix::dispatch(form_event &ev)
{
  element_set::iterator ii, endii;
  bool b;

  b = parent_type::dispatch(ev);    // self check

  endii = m_elements.end();
  for (ii=m_elements.begin(); !b && ii != endii; ++ii)
    b = ii->el->dispatch(ev);

  return b;
}

bool element_matrix::reparent(element *olde, element *newe)
{
  element_set::iterator ii, endii;

  // first pass, check if I have it

  endii = m_elements.end();
  for (ii=m_elements.begin(); ii != endii; ++ii)
    if (ii->el == olde) {
      // found it!
      ii->el = newe;
      return true;
    }

  // second pass, recurse

  endii = m_elements.end();
  for (ii=m_elements.begin(); ii != endii; ++ii)
    if ( ii->el->reparent(olde,newe) )
      return true;

  return false;
}

element * element_matrix::add_element(size_t x, size_t y, size_t spanw, size_t spanh, element *el)
{
  size_t ix, iy;

  assert(spanw>0);
  assert(spanh>0);

  m_elements(x,y).el = el;
  m_elements(x,y).w = spanw;
  m_elements(x,y).h = spanh;
  m_elements(x,y).active = true;

  // puch out actives through the spans
  for (iy=0; iy<spanh; ++iy)
    for (ix=0; ix<spanw; ++ix)
      m_elements(x+ix, y+iy).active = true;

  return el;
}

//
//
// error_list
//
//


error_list::error_list(void)
{
}

void error_list::render(scopira::tool::oflow_i &out)
{
  if (m_errors.empty())
    return;   // do nothing

  list_type::iterator ii, endii;

  endii = m_errors.end();
  out << "Errors:<UL>\n";
  for (ii=m_errors.begin(); ii != endii; ++ii)
    out << "<LI>" << *ii << "</LI>\n";
  out << "</UL>\n";
}

void error_list::push_back(const std::string &err)
{
  m_errors.push_back(err);
}

