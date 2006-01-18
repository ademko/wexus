
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

#include <wexus/ui/formreactor.h>

#include <scopira/tool/util.h>
#include <wexus/core/front.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus;
using namespace wexus::ui;

//
//
// form_session
//
//

form_session::form_session(void)
{
}
#include <scopira/tool/output.h>
bool form_session::handle_form_event(form_event &ev, oflow_i &out)
{
  locker L(m_element_lock);

  if (m_cur_element.get()) {
    if (!ev.get_command().empty()) {
      // dispatch this command
      m_cur_element->dispatch(ev);

      // check if we need to do a replace operation
      switch (ev.m_op) {
        case form_event::op_push_c:
          // save it for the pop
          m_pairs.push_back(epair_t(ev.m_replace_new, ev.m_replace_old));
          // fall through
        case form_event::op_replace_c:
          if (ev.m_replace_old == m_cur_element)
            { m_cur_element = ev.m_replace_new; OUTPUT << "weifjwef\n"; }
          else
            element_reparent(ev.m_replace_old.get(), ev.m_replace_new.get());  // do reparent operation
          break;
        case form_event::op_pop_c: {
            pairs_t::iterator ii, endii;
            endii = m_pairs.end();
            for (ii=m_pairs.begin(); ii != endii; ++ii)
              if (ii->first == ev.m_replace_old) {
                if (ii->first == m_cur_element)
                  m_cur_element = ii->second;
                else
                  element_reparent(ev.m_replace_old.get(), ii->second.get());

                // remove it from the list
                m_pairs.erase(ii);
                break;
              }
          }
          break;
        case form_event::op_none_c:
          // nothing
          break;
      }
    }
    // render whatever we have left
    m_cur_element->render(out);
    return true;
  } else
    return false;
}

void form_session::set_current_element(element *el)
{
  locker L(m_element_lock);

  m_cur_element = el;
}

void form_session::element_reparent(element *olde, element *newe)
{
  // assumes im in a lock area, ofcourse
  if (olde == m_cur_element.get())
    m_cur_element = newe;    // easy case
  else
    m_cur_element->reparent(olde, newe);  // recursive reparent
}

//
//
// form_reactor
//
//

form_reactor::form_reactor(core::session_manager *sesmgr,
  form_factory_i *fact, db::connection_pool *dbpoolmgr)
  : m_sesmgr(sesmgr), m_dbpoolmgr(dbpoolmgr), m_factory(fact), m_trigger("form")
{
  assert(fact);
  assert(sesmgr);
}

bool form_reactor::handle_app_event(core::app_event &evt)
{
  if (evt.req != m_trigger)
    return false;

  int tag;
  form_session *fses;

  // get the tag
  if (!string_to_int(evt.get_form_field("t"), tag))
    tag = 0;

  form_event fe(tag, &evt.get_form_field("c"), &evt, m_dbpoolmgr);

  // gets the session
  fses = dynamic_cast<form_session*>(m_sesmgr->get_session(evt.front));
  // no session? make one!
  if (!fses) {
    fses = new form_session;
    fses->set_current_element(m_factory->make_form());
    m_sesmgr->set_session(evt.front, fses);
  }

  // let the form session handle this thing
  fses->handle_form_event(fe, evt.front.get_output());

  // explicit cleanup, ugh

  return true;
}

