
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

#include <wexus/core/filemanager.h>

#include <scopira/tool/fileflow.h>
#include <scopira/tool/diriterator.h>
#include <scopira/tool/util.h>
#include <scopira/tool/time.h>
#include <scopira/tool/file.h>
#include <scopira/tool/prop.h>

#include <wexus/core/http.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus;

//core::file_manager
core::file_manager::file_manager(bool indexdirs, bool indexfile)
  : m_indexdir(indexdirs), m_indexfile(indexfile)
{
  m_throttle = 0;
}

core::file_manager::~file_manager()
{
}

bool core::file_manager::init_prop(scopira::tool::property *prop)
{
  property::iterator jj;
  const std::string *thr;

  for (jj=prop->get("filedir"); jj.valid(); ++jj)
    add_directory((*jj)->get_value_as_string());

  thr = prop->get_string_value("throttle");
  if (thr)
    m_throttle = string_to_int(*thr);

  return true;
}

void core::file_manager::add_directory(const std::string &dir)
{
  m_dirlist.push_back(dir);
}

int core::file_manager::find_file(const std::string &filename, std::string &fullpath) const
{
  file fname_info;

  for (dirlist_t::const_iterator ii=m_dirlist.begin(); ii != m_dirlist.end(); ++ii) {
    fullpath = *ii + dir_seperator_c + filename;
    fname_info.set(fullpath);

    if (fname_info.is_dir()) {
      std::string indexfilename(fullpath + dir_seperator_c + "index.html");
      if (!filename.empty() && *(filename.rbegin()) != '/') {
        // we find a directory, but the request doesnt end in a /
        // we should redirect the user
        return needslash_c; // needs an additional /
      } if (m_indexfile && file(indexfilename).exists()) {
        fullpath = indexfilename;
        return index_c; //found the index file
      } else if (m_indexdir) {
        fullpath += '/';
        return listing_c;    // its a directory without an index file, go ahead and dynamically index it
      } else
        return notfound_c;   // its a directory, but we cant do anything about it
    } else if (fname_info.exists())
      return file_c; // its a normal file
  }//master for

  return notfound_c; // couldnt find anything
}

void core::file_manager::send_index(const std::string &filename, const std::string &fullpath, scopira::tool::oflow_i &out) const
{
  // do directory listing/indexing
  dir_iterator di;
  std::string basedir(fullpath + dir_seperator_c), curname, curname_enc;

  out << "<h2>Index of: " << filename <<
    "</h2><UL><LI><A HREF=\"../\">Parent Directory</A><P>\n";
  di.open(fullpath);
  while (di.next(curname)) {
    assert(!curname.empty());
    if (curname[0] != '.') {
      file curinfo(basedir + curname);

      core::url_encode(curname, curname_enc);
      out << "<LI><A HREF=\"" << curname_enc;
      if (curinfo.is_dir())
        out << '/';
      out << "\">" << curname << "</A>  ";
      if (curinfo.is_dir())
        out << "(DIRECTORY)";
      else
        out << curinfo.size()/1024 << " kbytes";
      out << '\n';
    }
  }
  out << "</UL>\n";
}

