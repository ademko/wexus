
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

#include <wexus/core/checkers.h>

#include <scopira/tool/platform.h>

#ifdef PLATFORM_UNIX
#include <ctype.h>
#endif

//BBtargets libwexus.so

using namespace wexus;

core::checkers::checkers(void)
  : m_email("^[^[:space:]]+\\@[^[:space:]]+\\.[[:alpha:]]{2,3}$"),
    m_username("^[[:alnum:]]+$")
{
}

bool core::checkers::no_spaces(const std::string &s) const
{
  return s.find(" ") == std::string::npos;
}

bool core::checkers::only_alphanum(const std::string &s) const
{
  std::string::const_iterator ii, endii;

  endii = s.end();
  for (ii=s.begin(); ii != endii; ++ii)
    if (!isalnum(*ii))
      return false;
  return true;
}

bool core::checkers::postal_code(std::string &s) const
{
  return true;
}

bool core::checkers::phone(std::string &s) const
{
  return true;
}

bool core::checkers::email(const std::string &s) const
{
  return m_email.match(s);
}

bool core::checkers::username(const std::string &s, int min, int max) const
{
  return min_length(s, min) && max_length(s, max) && m_username.match(s);
}

bool core::checkers::password(const std::string &s, int min, int max) const
{
  return min_length(s, min) && max_length(s, max);
}

bool core::checkers::filename(const std::string &s) const
{
  return true;
}

