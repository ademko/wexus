
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

#include <wexus/core/reg_types.h>

#include <scopira/core/register.h>

#include <wexus/core/quitfront.h>
#include <wexus/core/scriptfront.h>

#include <wexus/core/httpfront.h>

//BBtargets libwexus.so

static scopira::core::register_flow<wexus::core::scriptfront> r1("wexus::core::scriptfront");
static scopira::core::register_flow<wexus::core::file_quitfront> r2("wexus::core::file_quitfront");
static scopira::core::register_flow<wexus::core::http_front> r3("wexus::core::http_front");

