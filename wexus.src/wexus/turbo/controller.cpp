
/*
 *  Copyright (c) 2006    Aleksander B. Demko
 *
 *  All rights reserved.
 *
 *  This material is confidential and proprietary information of
 *  Aleksander B. Demko ("Confidential Information").
 *  This Confidential Information may only be used and reproduced
 *  in accordance with the terms of the license agreement.
 *
 */

#include <wexus/turbo/controller.h>

#include <wexus/turbo/event.h>

//BBtargets libwexus.so

using namespace wexus::turbo;

//
//
// turbo_controller
//
//

turbo_controller::turbo_controller(void)
{
}

turbo_controller::~turbo_controller()
{
}

turbo_controller::standard_header::standard_header(const std::string &pagetitle)
{
  EVENT.output() << "<html><head><title>"
    << pagetitle << "</title></html><body><h1>" << pagetitle << "</h1><hr/>\n\n";
}

turbo_controller::standard_header::~standard_header()
{
  EVENT.output() << "\n\n<hr/></body></html\n\n";
}

