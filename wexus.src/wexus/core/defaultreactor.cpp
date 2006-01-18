
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

#include <wexus/core/defaultreactor.h>

#include <wexus/core/app.h>

//BBtargets libwexus.so

using namespace wexus;

//core::default_reactor
core::default_reactor::default_reactor(const std::string &defreq, reactor_i *defreactor)
  : m_defreq(defreq), m_defreactor(defreactor)
{
  assert(m_defreactor);
}

core::default_reactor::~default_reactor()
{
}

bool core::default_reactor::handle_app_event(app_event &evt)
{
  // make a new, copy event
  app_event myevt(m_defreq, evt.front, evt.app);

  // let the default reactor have it
  m_defreactor->handle_app_event(myevt);

  return true;
}

