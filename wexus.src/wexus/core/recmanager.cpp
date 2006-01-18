
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

#include <wexus/core/recmanager.h>

#include <scopira/tool/fileflow.h>
#include <scopira/tool/util.h>
#include <scopira/tool/file.h>

#include <wexus/core/output.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus;

//core::rec_manager
core::rec_manager::rec_manager(void)
{
}

core::rec_manager::~rec_manager()
{
}

bool core::rec_manager::init_prop(scopira::tool::property *prop)
{
  property::iterator jj;

  for (jj=prop->get("recdir"); jj.valid(); ++jj)
    add_directory((*jj)->get_value_as_string());

  return true;
}

void core::rec_manager::add_directory(const std::string &dir)
{
  m_dirlist.push_back(dir);
}

bool core::rec_manager::merge_template(const std::string &filename,
      wexus::rec::rec_i &root, scopira::tool::oflow_i &out)
{
  rec::token_i *tok;

  tok = find_token_tree(filename);
  if (tok) {
    rec::merge_rec_and_token(root, *tok, out);
    delete tok;
  }
  return tok != 0;
}

wexus::rec::token_i * core::rec_manager::find_token_tree(const std::string &filename)
{
  dirlist_t::iterator ii;
  std::string fname;

  for (ii=m_dirlist.begin(); ii != m_dirlist.end(); ++ii)
    if (file( (fname = *ii + dir_seperator_c + filename) ).exists()) {
      fileflow fflow(fname, fileflow::input_c);
      rec::token_i * tok;

      if (fflow.failed())
        return 0;   // failed to open?

      // tokenize the file
      tok = rec::text_tokenizer()(fflow);
      assert(tok);
      fflow.close();

      return tok;
    }

  // not found
  return 0;
}

core::cache_rec_manager::cache_rec_manager(void)
{
}

core::cache_rec_manager::~cache_rec_manager(void)
{
}

bool core::cache_rec_manager::merge_template(const std::string &filename,
      wexus::rec::rec_i &root, scopira::tool::oflow_i &out)
{
  // read lock ON (for the whole method, yes)
  cachemap_t::iterator ii;

  {
    read_locker kano(m_cacherw);

    ii = m_cache.find(filename);
    if (ii != m_cache.end()) {
      // cache hit
      rec::merge_rec_and_token(root, (*ii).second.ref(), out);
      return true;
    }
  }//read_locker

  // cache miss
  {
    write_locker lukang(m_cacherw);
    rec::token_i *tok;

    // we have to recheck, incase someone already did stuff it
    // between before we got the write lock (very doubtfull, but
    // could happen)

    ii = m_cache.find(filename);
    if (ii != m_cache.end()) {
      // cache hit, za?
      rec::merge_rec_and_token(root, (*ii).second.ref(), out);
      return true;
    }

    tok = find_token_tree(filename);

    if (tok) {
      // add tok to cache
      m_cache[filename] = tok; //cache now owns it, no need to delete
      // do the merge
      rec::merge_rec_and_token(root, *tok, out);
    }//if-tok

    return tok != 0;
  }//write_locker
}

void core::cache_rec_manager::flush(void)
{
  write_locker subzero(m_cacherw);

  m_cache.clear();
}

