
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

#include <wexus/core/app.h>

#include <wexus/core/front.h>
#include <wexus/core/string.h>

//BBtargets libwexus.so

using namespace wexus;

core::app_event_data::app_event_data(const std::string &oreq)
  : m_origreq(oreq)
{
}

const std::string & core::app_event::get_form_field(const std::string &fieldname) const
{
  if (front.has_form_field(fieldname))
    return front.get_form_field(fieldname);
  else
    return EMPTYSTRING;
}

core::app::app(void)
{
  // nothin yet
}

core::app::~app()
{
  // nothin yet
}

bool core::app::init_prop(const std::string &fist_mntpt, scopira::tool::property *prop)
{
  return true;
}

void core::app::start(void)
{
}

void core::app::notify_stop(void)
{
}

void core::app::wait_stop(void)
{
}

bool core::app::is_running(void) const
{
  return false;
}

void core::app::handle_app_event(app_event &evt)
{
  reactors_t::iterator ii;

  for (ii = m_reactors.begin(); ii != m_reactors.end(); ++ii)
    if ( (*ii)->handle_app_event(evt) )
      return; // found one that handled it, do nomore
}

void core::app::add_reactor(reactor_i *rea)
{
  m_reactors.push_back(rea);
}

