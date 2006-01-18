
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

#ifndef __INCLUDED_WEXUS_CORE_CHECKER_HPP__
#define __INCLUDED_WEXUS_CORE_CHECKER_HPP__

#include <string>

#include <scopira/tool/regex.h>

namespace wexus
{
  namespace core
  {
    class checkers;
  }
}

/**
 * data checkers
 * @author Aleksander Demko
 */ 
class wexus::core::checkers
{
  public:
    /// ctor
    checkers(void);

    /// true if not empty
    bool not_empty(const std::string &s) const { return !s.empty(); }
    /// true if has no spaces
    bool no_spaces(const std::string &s) const;
    /// only alphanumerics
    bool only_alphanum(const std::string &s) const;
  
    /// true if >=min length
    bool min_length(const std::string &s, int len) const { return s.size() >= len; }
    /// true is <=max length
    bool max_length(const std::string &s, int len) const { return s.size() <= len; }

    /// true if valid postal code (yes, non-const string) (Not yet implemented, returns true)
    bool postal_code(std::string &s) const;
    /// valid phone number (Not yet implemented, returns true)
    bool phone(std::string &s) const;

    /// true if valid email (*@*.??[?])
    bool email(const std::string &s) const;
    /// true if valid username (alpha-num only)
    bool username(const std::string &s, int min=4, int max=20) const;
    /// true if valid password
    bool password(const std::string &s, int min=4, int max=12) const;

    /// true if valid filename (Not yet implemented, returns true)
    bool filename(const std::string &s) const;

  private:
    scopira::tool::regex m_postal_code, m_phone, m_email, m_username, m_filename;
};

#endif

