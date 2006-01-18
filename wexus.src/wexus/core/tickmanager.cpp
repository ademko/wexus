
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

#include <wexus/core/tickmanager.h>

//BBtargets libwexus.so

using namespace wexus;

core::function_ticker_0::function_ticker_0(ticker t)
  : m_t(t)
{
}

void core::function_ticker_0::handle_tick(tick_event &evt)
{
  m_t();
}

core::function_ticker_1::function_ticker_1(ticker t)
  : m_t(t)
{
}

/// handles a tick
void core::function_ticker_1::handle_tick(tick_event &evt)
{
  m_t(evt);
}

core::tick_manager::tick_manager(void)
  : scopira::tool::thread(0), m_alive(true)
{
}

core::tick_manager::~tick_manager(void)
{
}

void core::tick_manager::notify_stop(void)
{
  m_alive = false;
}

void core::tick_manager::run(void)
{
  int minute, k;
  tickvec_t::iterator ii, endii;

  endii = m_ticks.end();
  minute = 0;
  while (m_alive) {
    // wait a minute, checkin every 6 sec if we should quit
    for (k=0; k<10; ++k) {
      sleep(6000);
      if (!m_alive)
        return;
    }

    // perhaps in the future add the ability for the manager to
    // detect which things it ran, and how long it ran things and
    // modify the wait time appropriatly. that way, all events
    // can happen in sync

    {
      core::tick_event evt;

      for (ii=m_ticks.begin(); ii!=endii; ++ii)
        if (minute % (*ii).first == 0)
          (*ii).second->handle_tick(evt);
    }

    // inc for next minute
    minute++;
  }
}

void core::tick_manager::add_ticker(int freq, ticker_i *t)
{
  assert(freq > 0);
  assert(t);

  m_ticks.push_back(tickpair_t(freq, t));
}

