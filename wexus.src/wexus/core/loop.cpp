
/*
 *  Copyright (c) 2005    Aleksander B. Demko
 *
 *  All rights reserved.
 *
 *  This material is confidential and proprietary information of
 *  Aleksander B. Demko ("Confidential Information").
 *  This Confidential Information may only be used and reproduced
 *  in accordance with the terms of the license agreement.
 *
 */

#include <wexus/core/loop.h>

//BBtargets libwexus.so

using namespace wexus::core;

wexus_loop::wexus_loop(int &argc, char **&argv)
  : basic_loop(argc, argv)
{
}

wexus_loop::~wexus_loop()
{
}

