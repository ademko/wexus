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

namespace wexus
{
  namespace core
  {
    class smtp_socket;
    class smtp;
  }
}

/**
 * a socket class specifically for the smtp protocol
 *
 * @author Andrew Kaspick
 */
class wexus::core::smtp_socket : public scopira::tool::netflow
{
  public:
    /// server reply - <status code, message>
    typedef std::pair<int, std::string> reply_t;
    /// a collection of replies
    typedef std::vector<reply_t> replies_t;

    smtp_socket(void) {}
    smtp_socket(const std::string& server, int port=25);
    virtual ~smtp_socket(void);

    /**
     * Sends a command to the smtp server.  All commands will have
     * a CRLF appended to them, so there is no need to include one
     * explicity in the command string.
     * 
     * @param command the command to send to the server
     * @author Andrew Kaspick
     */ 
    virtual bool send(const std::string& command);
    /**
     * Receives the reply from the server.
     * 
     * @param command the command to send to the server
     * @author Andrew Kaspick
     */ 
    virtual bool receive(replies_t& r);

  protected:
    bool parse_reply(const std::string& message, replies_t& r) const;
};

/**
 * smtp class to implement a subset of the protocol
 * in order to send simple email messages.
 *
 * @author Andrew Kaspick
 */
class wexus::core::smtp
{
  public:
    /// list of email addresses
    typedef std::vector<std::string> list_t;
    /// list of all email addresses (both To and Cc)
    struct mail_list_t
    {
      list_t to;  // addresses for the "To" field
      list_t cc;  // addresses for the "Cc" field
    };

    // specify 'to' field (nothing or something else)
    // bcc?

    /**
     * smtp constructor
     *
     * @param server the smtp server used to send the email
     * @param port the port to use to send the email (defaults to 25)
     * @author Andrew Kaspick
     */ 
    smtp(const std::string& server, int port=25);
    virtual ~smtp(void);

    /**
     * sends mail
     *
     * @param from the address the mail is from
     * @param to the address the mail is to
     * @param subject the subject of the mail message
     * @param data the actual message(data) to send in the email
     *
     * @return true if the mail was sent
     * @author Andrew Kaspick
     */ 
    bool send_mail(const std::string& from, const std::string& to, const std::string& subject, const std::string& data);
    /**
     * an advancded send mail with additional options
     *
     * @param from the address the mail is from
     * @param list a list that contains all the addresses that the mail will be sent 'to' and 'cc'ed to
     * @param subject the subject of the mail message
     * @param data the actual message(data) to send in the email
     * @param from_field an additional string to be used with the 'from' address (ex. Name <address@place.ca>)
     *
     * @return true if the mail was sent
     * @author Andrew Kaspick
     */ 
    bool send_mail(const std::string& from, const mail_list_t& list, const std::string& subject, const std::string& data, const std::string& from_field="");

  private:
    void send_list(const list_t& list, const std::string& field);

    wexus::core::smtp_socket m_server;  // the server to send mail through
};

#ifdef DEBUG_SMTP
scopira::tool::oflow_i& operator<<(scopira::tool::oflow_i& o, const wexus::core::smtp_socket::replies_t& r);
#endif

#endif

