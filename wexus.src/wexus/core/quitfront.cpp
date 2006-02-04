
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

#include <wexus/core/quitfront.h>

#include <scopira/tool/file.h>
#include <scopira/tool/flow.h>
#include <scopira/tool/prop.h>

#include <wexus/core/output.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus;

bool core::test_filequit(const std::string &filename)
{
  file nfo(filename);

  return nfo.exists() && nfo.size() == 0;
}

core::file_quitfront::file_quitfront(void)
  : thread(0), m_alive(true)
{
}

core::file_quitfront::file_quitfront(front_peer_i *peer, const std::string &filename)
  : thread(0), front_i(peer), m_alive(true), m_filename(filename)
{
  assert(peer);
}

core::file_quitfront::~file_quitfront()
{
}

bool core::file_quitfront::init_prop(front_peer_i *peer, scopira::tool::property *prop)
{
  const std::string *sptr;

  set_peer(peer);

  sptr = prop->get_string_value("filename");
  if (!sptr) {
    COREOUT << "Missing \"filename\" key for file_quitfront\n";
    return false;
  }

  m_filename = *sptr;
  return true;
}

void core::file_quitfront::notify_stop(void)
{
  m_alive = false;
}

void core::file_quitfront::run(void)
{
  while (m_alive) {
    sleep(10000);

    if (!test_filequit(m_filename)) {
      COREOUT << "file_quitfront " << m_filename << " nonzero_quitting\n";
      m_alive = false;
      get_peer()->handle_shutdown();
    }
  }
}

