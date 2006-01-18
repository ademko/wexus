
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

#include <wexus/core/scriptfront.h>

#include <assert.h>

#include <wexus/core/output.h>
#include <scopira/tool/flow.h>

//BBtargets libwexus.so

using namespace wexus;

core::scriptfront::scriptfront(void)
  : thread(0)
{
}

core::scriptfront::scriptfront(const std::string& filename, front_peer_i* peer)
  : front_i(peer), thread(0), m_filename(filename)
{
}

core::scriptfront::~scriptfront()
{
  wait_stop();
}

bool core::scriptfront::init_prop(front_peer_i *peer, scopira::tool::property *prop)
{
  set_peer(peer);

  // this class doesnt do anything :)
  return false;
}

void core::scriptfront::run(void)
{
  script_front_event ev;

  front_peer_i* peer = get_peer();

  assert(peer);

  COREOUT << "Script-sleep\n";
  scopira::tool::thread::sleep(3000);
  COREOUT << "Script-running\n";
  ev.set_request("url-one");
  peer->handle_front_event(ev);
  ev.set_request("url-two");
  peer->handle_front_event(ev);
  COREOUT << "Script-killing\n";
}

scopira::tool::oflow_i& core::scriptfront::script_front_event::get_output(void)
{
  return COREOUT;
}

