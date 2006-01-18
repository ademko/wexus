
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

#include <wexus/core/recreactor.h>

#include <scopira/tool/fileflow.h>
#include <scopira/tool/file.h>

#include <wexus/core/app.h>
#include <wexus/core/front.h>

#include <wexus/rec/token.h>

//BBtargets libwexus.so

using namespace wexus;

//core::rec_reactor
core::rec_reactor::rec_reactor(wexus::rec::rec_i *rootrec, wexus::core::rec_manager *mgr,
    const std::string &mimetype)
  : m_rootrec(rootrec), m_mgr(mgr), m_mimetype(mimetype)
{
  assert(m_rootrec);
  assert(m_mgr);
}

core::rec_reactor::~rec_reactor()
{
}

bool core::rec_reactor::handle_app_event(app_event &evt)
{
  // if the request contains illegal chars, abort
  if (!scopira::tool::file::check_path(evt.req))
    return false;

  return m_mgr->merge_template(evt.req, *m_rootrec, evt.front.get_output());
}

