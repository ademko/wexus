
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

#include <wexus/core/filereactor.h>
#include <wexus/core/app.h>
#include <wexus/core/engine.h>
#include <wexus/core/front.h>
#include <wexus/core/filemanager.h>

#include <scopira/tool/file.h>

//BBtargets libwexus.so

using namespace wexus;
using namespace scopira::tool;

core::file_reactor::file_reactor(wexus::core::file_manager *mgr)
  : m_mgr(mgr)
{
  assert(m_mgr);
}

core::file_reactor::~file_reactor()
{
  //nothing yet
}
 
bool core::file_reactor::handle_app_event(app_event &evt)
{
  int ret;

  // if the request contains illegal chars, abort
  if (!file::check_path(evt.req))
    return false;

  // find the file and send it
  //tool::time mod_time;
  std::string fullpath;
  ret = m_mgr->find_file(evt.req, fullpath);

  // check if we need to do anything special
  switch (ret) {
    case file_manager::notfound_c:
      return false;
    case file_manager::needslash_c:
      evt.front.set_redirect_url(evt.app.get_orig_req() + "/");
      return true;
    case file_manager::listing_c:
      m_mgr->send_index(evt.req, fullpath, evt.front.get_output());
      return true;
  }
  // we know we have file_manager::file_c or index_c

  // set its mime type
  std::string::size_type i;
  std::string fileext("bin"), mimetype;

  // get the file extention from the request.
  // Algorithm: start at the end and look for first '.'
  // if we find a '/' before a '.' then the request is incorrect
  // allowed request example = blah.jpg
  // not allowed request example = blah.jpg/blah
  if ( (fullpath.size() > 1) && (fullpath[fullpath.size()-1] != '/')
      && (fullpath[fullpath.size()-1] != '.') )
  {
    for (i=fullpath.size()-1; i>0; --i)
    {
      if (fullpath[i] == '/')
        break;
      else if (fullpath[i] == '.')
      {
        // search will stop on the '.' so skip the '.'
        fileext = fullpath.substr(i+1, fullpath.size());
        break;
      }
    }
  }

  // file is good, get/set the content type
  core::engine::instance()->get_mimetype_manager().get_type(fileext, mimetype);
  evt.front.set_content_type(mimetype);

  // set modification time on event
  //evt.front.set_modified_time(mod_time);
  evt.front.send_file(fullpath, m_mgr->get_throttle());

  return true;
}

