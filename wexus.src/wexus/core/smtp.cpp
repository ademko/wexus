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
#include <wexus/core/output.h>

//BBtargets libwexus.so

using namespace wexus::core;
using namespace scopira::tool;

smtp_socket::smtp_socket(const std::string& server, int port)
{
  nethostrec rec;
  hostname_to_hostrec(server, rec);

  open(&rec.get_addr(), port);
}

smtp_socket::~smtp_socket()
{
  close();
}

bool smtp_socket::send(const std::string& command)
{
  // send the command
  *this << command+"\r\n";

#ifdef DEBUG_SMTP
  COREOUT << "S: "+command+"\r\n";
#endif

  return !failed();
}

bool smtp_socket::receive(replies_t& r)
{
  static const int max_size = 1024;
  bufferflow::byte_buffer_t buf;
  buf.reserve(max_size);
  buf.resize(max_size);
  std::string reply;

  // wait and make sure we have a connection
  // wait 30 seconds
  if (!read_ready(30*1000))
  {
#ifdef DEBUG_SMTP
    COREOUT << "error: can't connect to server\n";
#endif
    return false;
  }

  int size;
  // get the servers replies
  while (read_ready(0) && (size = read(&buf[0], buf.size())) > 0)
  {
    buf.resize(size);
    reply.append(buf.begin(), buf.end());
    buf.resize(max_size);
  }

  if (reply.empty())
    return false;

  // parse the replies
#ifdef DEBUG_SMTP
  bool test = parse_reply(reply, r);
  COREOUT << "R: " << r;
  return test;
#else
  return parse_reply(reply, r);
#endif
}

bool smtp_socket::parse_reply(const std::string& message, replies_t& r) const
{
  // clear the collection
  r.clear();

  std::vector<std::string> responses;
  // each reply is seperated by a CRLF
  scopira::tool::string_tokenize_word(message, responses, "\r\n");

  for (std::vector<std::string>::iterator it=responses.begin(); it!=responses.end(); it++)
  {
    // parse the reply into it's status code and response message
    std::string left, right;
    scopira::tool::split_char(*it, ' ', left, right);
    if (left.empty() || right.empty())
      return false;

    // insert the pair
    r.push_back(std::make_pair(scopira::tool::string_to_int(left), right));
  }

  return true;
}


smtp::smtp(const std::string& server, int port)
{
  nethostrec rec;
  hostname_to_hostrec(server, rec);

  m_server.open(&rec.get_addr(), port);
}

smtp::~smtp(void)
{
  m_server.close();
}

bool smtp::send_mail(const std::string& from, const std::string& to, const std::string& subject, const std::string& data)
{
  mail_list_t list;
  list.to.push_back(to);

  return send_mail(from, list, subject, data);
}

bool smtp::send_mail(const std::string& from, const mail_list_t& list, const std::string& subject, const std::string& data, const std::string& from_field)
{
  smtp_socket::replies_t r;

  // receive initial connection message
  if (!m_server.receive(r))
    return false;

  std::string cmd;

  // identify self 
  cmd = "HELO "+get_hostname();
  m_server.send(cmd);
  // receive welcome response
  m_server.receive(r);

  // tell server who the message is from
  cmd = "MAIL FROM:<"+from+">";
  m_server.send(cmd);
  m_server.receive(r);

  // send the list of addresses that the message is for
  for (list_t::const_iterator it=list.to.begin(); it!=list.to.end(); it++)
  {
    cmd = "RCPT TO:<"+*it+">";
    m_server.send(cmd);
    m_server.receive(r);
  }

  cmd = "DATA";
  // send the data command
  m_server.send(cmd);
  m_server.receive(r);

  // send headers
  if (from_field.empty())
    m_server.send("From: "+from);
  else
    m_server.send("From: "+from_field+" <"+from+">");

  send_list(list.to, "To");
  send_list(list.cc, "Cc");

  m_server.send("Subject: "+subject);

  // send the data
  if (!data.empty())
  {
    // if a . is found at the start then insert an additional . at the beginning.
    // (see sec. 4.5.2. of RFC0821)
    if (data[0] == '.')
      m_server.send('.'+data);
    else
      m_server.send(data);
  }
  // end data transfer with a . on a line by itself
  m_server.send(".");
  m_server.receive(r);

  // send quit command
  cmd = "QUIT";
  m_server.send(cmd);

  // receive close message
  m_server.receive(r);

  return !m_server.failed();
}

void smtp::send_list(const list_t& list, const std::string& field)
{
  for (list_t::const_iterator it=list.begin(); it!=list.end(); it++)
    m_server.send(field+": "+*it);
}

#ifdef DEBUG_SMTP
oflow_i& operator<<(oflow_i& o, const smtp_socket::replies_t& r)
{
  for (smtp_socket::replies_t::const_iterator it=r.begin(); it!=r.end(); it++)
    COREOUT << int_to_string((*it).first) << " " << (*it).second << "\n";

  return o;
}
#endif

#ifdef CORE_DEBUG_SMTP

int main(int argc, char** argv)
{
  output = new fileflow(fileflow::stdout_c, 0);

  netflow::init();

  smtp mail("mailserver");
  smtp::mail_list_t list;
  list.to.push_back("blah@anon.com");
  list.cc.push_back("anon@anon.com");
  mail.send_mail("bob@dude.poo", list, "test", "Test Message", "Test");

  netflow::cleanup();

  return 0; //return ok
}

#endif
