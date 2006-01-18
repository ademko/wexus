
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

#include <wexus/core/listreactor.h>

#include <wexus/core/app.h>

//BBtargets libwexus.so

using namespace wexus;

//core::list_reactor
core::list_reactor::list_reactor(void)
{
}

core::list_reactor::~list_reactor()
{
}

bool core::list_reactor::handle_app_event(app_event &evt)
{
  rlist_t::iterator ii;

  for (ii=m_rlist.begin(); ii != m_rlist.end(); ++ii)
    if ( (*ii)->handle_app_event(evt) )
      return true;

  // noone took it?
  return false;
}

void core::list_reactor::add_reactor(reactor_i *r)
{
  m_rlist.push_back(r);
}

//********************************************
//
// ifmatch_reactor implementation
//
//********************************************

core::ifmatch_reactor::ifmatch_reactor(const std::string& match, int where, bool not_op)
  : m_matchstring(match), m_where(where), m_not_op(not_op)
{
}

core::ifmatch_reactor::ifmatch_reactor(const std::string& encoded_match)
{
  m_not_op = false;
  m_where = whole_c;

  if (encoded_match.empty())
    return;
  m_not_op = encoded_match[0] == '!';

  if (m_not_op && encoded_match.size() == 1)
    return; // lone "!", not even an op. bad.

  switch (encoded_match[1]) {
    case '<': m_where = begin_c; break;
    case '>': m_where = end_c; break;
    case '=': m_where = whole_c; break;
    default: m_where = subset_c;        // undocumented feature :)
  }

  m_matchstring = encoded_match.substr(m_not_op?1:0);
}

core::ifmatch_reactor::~ifmatch_reactor()
{
}

bool core::ifmatch_reactor::handle_app_event(app_event& evt)
{  
  if (m_matchstring.empty() && evt.req.empty())
    return list_reactor::handle_app_event(evt);
  else if (!m_matchstring.empty())
  {
    bool match = false;
    switch (m_where)
    {
      case whole_c:
        match = (evt.req == m_matchstring);
        break;
      case subset_c:
        match = (evt.req.find(m_matchstring) != std::string::npos);
        break;
      case begin_c:
        match = (evt.req.find(m_matchstring) == 0);
        break;
      case end_c:
        match = (evt.req.rfind(m_matchstring)+m_matchstring.size() == evt.req.size());
        break;
      default:
        assert(false); // undefined
        return false;
    }

    // xor... dont be so clever :)
    if (match == !m_not_op)
      return list_reactor::handle_app_event(evt);
  }

  return false;
}

