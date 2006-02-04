
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

#ifndef __INCLUDED_WEXUS_CORE_SMTP_HPP__
#define __INCLUDED_WEXUS_CORE_SMTP_HPP__

#include <vector>

#include <scopira/tool/netflow.h>
#include <scopira/tool/cacheflow.h>

namespace wexus
{
  namespace core
  {
    class smtp_session;
  }
}

/**
 * A session with a remote smtp server. This is used to send email.
 * This is a clean rewrite of the previous implementation.
 *
 * @author Aleksander Demko
 */ 
class wexus::core::smtp_session
{
  public:
    /// list of all email addresses (both To and Cc)
    typedef std::vector<std::string> addresses_t;    // list of email addresses

    /**
     * smtp constructor
     *
     * @param server the smtp server used to send the email
     * @param port the port to use to send the email (defaults to 25)
     * @author Aleksander Demko
     */ 
    smtp_session(const scopira::tool::netaddr &server, int port = 25);

    /// dtor
    ~smtp_session();

    /**
     * Is the session on afailed state (useful to check after construction)
     *
     * @author Aleksander Demko
     */
    bool failed(void) const { return m_failed; }

    /**
     * sends mail
     *
     * @param from the address the mail is from
     * @param to the address the mail is to
     * @param subject the subject of the mail message
     * @param data the actual message(data) to send in the email
     *
     * @return true if the mail was sent
     * @author Aleksander Demko
     */ 
    bool send_mail(const std::string& fromemail, const std::string& to, const std::string& subject, const std::string& data);
    /**
     * Full sender.
     *
     * @return true if the mail was sent
     * @author Aleksander Demko
     */ 
    bool send_mail(const std::string& fromemail, const addresses_t& tolist, const addresses_t &cclist,
      const std::string& subject, const std::string& data);

  private:
    bool recv_line(int &code, std::string &msg);
    bool send_line(const std::string &linedata);
    bool send_half_line(const std::string &linedata);
    bool send_data_lines(const std::string &msgbody);

    scopira::tool::netflow m_socket;    // the socket connection to the remote server
    bool m_failed, m_gotinitial;

    scopira::tool::circular_vector<char> m_input;
};

#endif

