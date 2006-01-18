
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

#ifndef __INCLUDED_WEXUS_CORE_WEBEVENT_HPP_H__
#define __INCLUDED_WEXUS_CORE_WEBEVENT_HPP_H__

#include <wexus/core/front.h>

namespace wexus
{
  namespace core
  {
    class http_event_imp;
  }
}

/**
 * a front event specifically for web interfaces
 *
 * @author Andrew Kaspick
 */
class wexus::core::http_event_imp : public virtual wexus::core::front_event_imp
{
  public:
    /// sends response line and header data
    virtual bool send_headers(void) = 0;
    /// sends body of the response
    virtual void send_body(void) = 0;

    /// creates an error page
    virtual void create_error_page(int status, const std::string& message);

  protected:
    /// sends all cookies set on the server to the client
    virtual void send_server_cookies(void);

    /// send an individual cookie
    virtual void send_server_cookie(const std::string& cookie) = 0;
};

#endif

