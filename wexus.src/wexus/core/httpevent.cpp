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

#include <wexus/core/httpevent.h>

#include <scopira/tool/flow.h>

//BBtargets libwexus.so

using namespace wexus;
using namespace wexus::core;

void http_event_imp::send_server_cookies(void)
{
  const server_cookies_t& cookies = get_server_cookies();

  // used to construct set-cookie header
  std::string cookie_hdr;

  // iterate over the paths in the multimap
  server_cookies_t::const_iterator it1;
  for (it1=cookies.begin(); it1!=cookies.end(); it1++)
  {
    // iterate over cookies for a given path
    server_cookies_t::const_iterator it2;
    for (it2=cookies.lower_bound((*it1).first); it2!=cookies.upper_bound((*it1).first); it2++)
    {
      const cookiemap_t& cookie = (*it2).second;

      // iterate over the values for the cookie
      cookiemap_t::const_iterator it3;
      for (it3=cookie.begin(); it3!=cookie.end(); it3++)
      {
        // send cookie information to client
        cookie_hdr = (*it3).first + "=" + (*it3).second.value + "; path=" + (*it1).first;
       
        if (!(*it3).second.domain.empty())
          cookie_hdr += "; domain=" + (*it3).second.domain;

        if (!(*it3).second.expires.empty())
          cookie_hdr += "; expires=" + (*it3).second.expires;

        send_server_cookie(cookie_hdr);
      }
    }
  }
}

void http_event_imp::create_error_page(int status, const std::string& message)
{
  // page is only necessary for error codes, which are 400 and above
  assert(status >= 400);

  if (status >= 400)
  {
    // customized error pages must be greater than 512 bytes in order for IE to not
    // display 'friendly' error page.  Can be any size for other browsers.
    // ##do something better with this in the future
    get_output() <<
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">"
      "<html>\n"
      "<head><title>" << message << "</title></head>\n"
      "<body bgcolor=\"#FFFFFF\" text=\"#000000\">\n"
      "<h2>ERROR " << message << "</h2>\n"
      "<p>                                                                  "
      "                                                                     "
      "                                                                     "
      "                                                                     "
      "                                                                     "
      "                                                                 </p>"
      "</body>\n"
      "</html>\n";
  }
}

