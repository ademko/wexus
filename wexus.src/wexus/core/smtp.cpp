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

#include <wexus/core/smtp.h>

#include <scopira/tool/bufferflow.h>
#include <scopira/tool/util.h>
#include <scopira/tool/fileflow.h>
#include <scopira/tool/array.h>

#include <wexus/core/output.h>

//BBtargets libwexus.so

//#define DEBUG_SMTP

// reference: RFC0821

using namespace wexus::core;
using namespace scopira::tool;

smtp_session::smtp_session(const scopira::tool::netaddr &server, int port)
{
  m_socket.open(&server, port);
  m_failed = m_socket.failed();
  m_gotinitial = false;
  m_input.reserve(4096);
}

smtp_session::~smtp_session()
{
  if (m_gotinitial && !m_socket.failed()) {
    int code;
    std::string msg;

    // lets be nice
    send_line("QUIT");
    recv_line(code, msg);
  }
}

bool smtp_session::send_mail(const std::string& fromemail, const std::string& to, const std::string& subject, const std::string& data)
{
  addresses_t list;
  list.push_back(to);

  OUTPUT << "smtp_session to=" << to << '\n';

  return send_mail(fromemail, list, addresses_t(), subject, data);
}

bool smtp_session::send_mail(const std::string& fromemail, const addresses_t& tolist, const addresses_t &cclist,
  const std::string& subject, const std::string& data)
{
  int code;
  std::string recvmsg;
  std::string cmd;

  recvmsg.reserve(128);
  cmd.reserve(128);

  if (!m_gotinitial) {
    m_gotinitial = true;
    if (!recv_line(code, recvmsg))
      return false;
  }

  send_line("HELO " + get_hostname());
  if (!recv_line(code, recvmsg))
    return false;

  send_line("MAIL FROM:<" + fromemail + ">");
  recv_line(code, recvmsg);

  for (int i=0; i<tolist.size(); ++i)
  {
    send_line("RCPT TO:<" + tolist[i] + ">");
    recv_line(code, recvmsg);
  }

  send_line("DATA");
  recv_line(code, recvmsg);

  send_line("From: " + fromemail);

  for (int i=0; i<tolist.size(); ++i)
    send_line("To: " + tolist[i]);
  for (int i=0; i<cclist.size(); ++i)
    send_line("Cc: " + cclist[i]);

  send_line("Subject: "+subject);

  send_line("");

  if (!data.empty())
    send_data_lines(data);

  send_line(".");
  recv_line(code, recvmsg);

  return !m_socket.failed();
}

bool smtp_session::recv_line(int &code, std::string &msg)
{
  std::string fullline;

  fullline.reserve(128);

#ifdef DEBUG_SMTP
  OUTPUT << "R: ";
#endif

  while (true) {
    if (m_input.empty()) {
      // get more data from the socket, as our buffer is empty
      scopira::tool::basic_array<char> buf;
      buf.resize(m_input.capacity());
      size_t readin;

#ifdef DEBUG_SMTP
  OUTPUT << "s";
#endif
      readin = m_socket.read_short(reinterpret_cast<scopira::tool::byte_t*>(buf.c_array()), buf.size());
#ifdef DEBUG_SMTP
  OUTPUT << readin;
#endif
      if (readin == 0)
        return false;

      // add it to the buffer
      m_input.push_back(buf.begin(), buf.begin() + readin);
    }

    char c = m_input.front();
    m_input.pop_front();

    fullline.push_back(c);

    if (c == '\n')
      break;
  }

#ifdef DEBUG_SMTP
  OUTPUT << "\" " << fullline << "\"\n";
#endif

  // ok, parse the results
  std::string left;
  scopira::tool::split_char(fullline, ' ', left, msg);

  if (!string_to_int(left, code))
    code = 1;

  return true;
}

bool smtp_session::send_line(const std::string &linedata)
{
#ifdef DEBUG_SMTP
  OUTPUT << "S: \"" << linedata;
#endif

  m_socket << linedata << "\r\n";

#ifdef DEBUG_SMTP
  OUTPUT << "\"\n";
#endif

  return !m_socket.failed();
}

bool smtp_session::send_half_line(const std::string &linedata)
{
#ifdef DEBUG_SMTP
  OUTPUT << "S: " << linedata;
#endif

  m_socket << linedata << "\n";

#ifdef DEBUG_SMTP
  m_socket << '\n';
#endif

  return !m_socket.failed();
}

bool smtp_session::send_data_lines(const std::string &msgbody)
{
  // lazy, but in the future, do this line by line
  std::vector<std::string> alllines;
  std::string linebuf;

  linebuf.reserve(128);

  string_tokenize_word(msgbody, alllines, "\n");

#ifdef DEBUG_SMTP
  OUTPUT << "PREPARING MSG OF PARTS#" << alllines.size() << '\n';
#endif

  for (int i=0; i<alllines.size(); ++i) {
    linebuf = alllines[i];

    if (linebuf.empty())
      { }  // nothing
    else {
      if (linebuf[0] == '.')    // 4.5.2 of the RFC
        linebuf.insert(0, ".");
      if (linebuf[linebuf.size() - 1] == '\r')
        linebuf.resize(linebuf.size() - 1);
    }

    send_line(linebuf);
  }

  return true;
}

