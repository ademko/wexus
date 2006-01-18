
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

#include <wexus/core/redirectreactor.h>

#include <scopira/tool/flow.h>

#include <wexus/core/front.h>
#include <wexus/core/app.h>

//BBtargets libwexus.so

using namespace wexus;

core::redirect_reactor::redirect_reactor(void)
{
}

core::redirect_reactor::redirect_reactor(const std::string &req, const std::string &url)
{
  add_url(req, url);
}

core::redirect_reactor::~redirect_reactor()
{
}

bool core::redirect_reactor::handle_app_event(app_event &evt)
{
  urlmap_t::const_iterator ii;

  ii = m_urlmap.find(evt.req);
  if (ii != m_urlmap.end()) {

    evt.front.set_redirect_url((*ii).second);

    return true;
  }

  return false;
}

void core::redirect_reactor::add_url(const std::string &req, const std::string &url)
{
  m_urlmap[req] = url;
}

