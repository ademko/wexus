
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

#include <wexus/core/mimetypemanager.h>

//BBtargets libwexus.so

using namespace wexus;

//core::mimetype_manager
core::mimetype_manager::mimetype_manager(void)
{
}

core::mimetype_manager::~mimetype_manager()
{
}

void core::mimetype_manager::add_default_types(void)
{
  // default mime types
  // todo: move these to properties file
  add_type("jpg", "image/jpeg");
  add_type("jpeg", "image/jpeg");
  add_type("gif", "image/gif");
  add_type("png", "image/png");
  add_type("txt", "text/plain");
  add_type("pdf", "application/pdf");
  add_type("zip", "application/zip");
  add_type("gz", "application/x-gzip");
  add_type("tgz", "application/x-gzip");
  add_type("html", "text/html");
  add_type("htm", "text/html");
}

void core::mimetype_manager::add_os_types(void)
{
  // under linux, this will read /etc/mime.types.
  // under win32, this will... read the registry?
}

void core::mimetype_manager::add_type(const std::string &ext, const std::string &mime_type)
{
  m_mimemap[ext] = mime_type;
}

void core::mimetype_manager::get_type(const std::string& ext, std::string& mime_type) const
{
  mimemap_t::const_iterator mimeiterator = m_mimemap.find(ext);

  if (mimeiterator == m_mimemap.end())
    mime_type = "application/octet-stream";
  else
    mime_type = (*mimeiterator).second;
}

