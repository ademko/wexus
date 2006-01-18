
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

#include <scopira/tool/objflowloader.h>
#include <scopira/tool/prop.h>
#include <scopira/tool/flow.h>

#include <wexus/core/reg_types.h>
#include <wexus/core/engine.h>
#include <wexus/core/output.h>

//BBlibs wexus
//BBtargets wexusd

#ifndef HAL_DEBUG_MAIN
int main(int argc, char **argv)
{
#ifndef NDEBUG
  scopira::tool::objrefcounter _duh;
#endif
  scopira::tool::objflowloader loader;
  wexus::core::engine vtec;
  scopira::tool::property_node props;

  // init rand seed
  srand(time(0));

  if (!vtec.load_prop_file(argc>1 ? argv[1] : "wexus.config", &props)) {
    COREOUT << "Error loading property file (wexus.config); ending Wexus/Daemon...\n";
    return 1;
  }

  COREOUT << "Starting Wexus/Daemon...\n";

  // start all the fronts
  vtec.start();

  // block here until engine dies naturally
  vtec.wait_stop();

  COREOUT << "Ending Wexus/Daemon...\n";

  return 0;
}
#endif

