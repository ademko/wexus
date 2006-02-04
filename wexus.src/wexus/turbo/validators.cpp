
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

#include <wexus/turbo/validators.h>

#include <wexus/turbo/event.h>
#include <wexus/turbo/session.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus::db;
using namespace wexus::turbo;

//
//
// check_form
//
//

check_form::check_form(void)
{
}

check_form::check_form(const std::string &formname)
  : m_formname(formname)
{
}

bool check_form::is_notnull(const std::string &fieldname)
{
  return EVENT.has_form_field(my_field(fieldname));
}

bool check_form::is_notempty(const std::string &fieldname)
{
  return !my_get_form_field(fieldname).empty();
}

bool check_form::is_integer(const std::string &fieldname)
{
  int dummy;
  return string_to_int(my_get_form_field(fieldname), dummy);
}

bool check_form::is_minlength(const std::string &fieldname, int minlength)
{
  return my_get_form_field(fieldname).size() >= minlength;
}

bool check_form::is_maxlength(const std::string &fieldname, int maxlength)
{
  return my_get_form_field(fieldname).size() <= maxlength;
}

bool check_form::is_email(const std::string &fieldname)
{
  scopira::tool::regex emailex("^[^[:space:]]+\\@[^[:space:]]+\\.[[:alpha:]]{2,5}$");   // how many chars can TLDs have no? more than 5?
  return emailex.match(my_get_form_field(fieldname));
}

bool check_form::is_username(const std::string &fieldname)
{
  scopira::tool::regex usernameex("^[[:alnum:]]+$");
  return usernameex.match(my_get_form_field(fieldname));
}

bool check_form::is_password(const std::string &fieldname)
{
  const std::string &s = my_get_form_field(fieldname);

  return s.size()>=4 && s.size()<=20;
}

bool check_form::check_notnull(const std::string &fieldname)
{
  bool ret = is_notnull(fieldname);

  if (!ret)
    EVENT.add_error(my_field(fieldname), fieldname + " must be provided");

  return ret;
}

bool check_form::check_notempty(const std::string &fieldname)
{
  bool ret = is_notempty(fieldname);

  if (!ret)
    EVENT.add_error(my_field(fieldname), fieldname + " cannot be empty");

  return ret;
}

bool check_form::check_integer(const std::string &fieldname)
{
  bool ret = is_integer(fieldname);

  if (!ret)
    EVENT.add_error(my_field(fieldname), fieldname + " must be a number");

  return ret;
}

bool check_form::check_minlength(const std::string &fieldname, int minlength)
{
  bool ret = is_minlength(fieldname, minlength);

  if (!ret)
    EVENT.add_error(my_field(fieldname), fieldname + " must be atleast " + int_to_string(minlength) + " characted in length");

  return ret;
}

bool check_form::check_maxlength(const std::string &fieldname, int maxlength)
{
  bool ret = is_maxlength(fieldname, maxlength);

  if (!ret)
    EVENT.add_error(my_field(fieldname), fieldname + " cannot be more than " + int_to_string(maxlength) + " characted in length");

  return ret;
}

bool check_form::check_email(const std::string &fieldname)
{
  bool ret = is_email(fieldname);

  if (!ret)
    EVENT.add_error(my_field(fieldname), fieldname + " doesn't look like a valid email address");

  return ret;
}

bool check_form::check_username(const std::string &fieldname)
{
  bool ret = is_username(fieldname);

  if (!ret)
    EVENT.add_error(my_field(fieldname), fieldname + " can only contain letters and numbers");

  return ret;
}

bool check_form::check_password(const std::string &fieldname)
{
  bool ret = is_password(fieldname);

  if (!ret)
    EVENT.add_error(my_field(fieldname), fieldname + " must be a valid password (4-20 characters in length)");

  return ret;
}

bool check_form::check(bool b, const std::string &msg)
{
  if (!b)
    EVENT.add_error(msg);

  return b;
}

bool check_form::get(const std::string &fieldname, wexus::db::dbint &out) const
{
  string_to_dbint(my_get_form_field(fieldname), out);
  return true;
}

bool check_form::get(const std::string &fieldname, wexus::db::dbstring &out) const
{
  out = my_get_form_field(fieldname);
  return true;
}

bool check_form::has_error(void)
{
  return EVENT.has_error();
}

std::string check_form::my_field(const std::string &fieldname) const
{
  if (m_formname.empty())
    return fieldname;
  else
    return m_formname + "[" + fieldname + "]";
}

const std::string & check_form::my_get_form_field(const std::string &fieldname) const
{
  if (m_formname.empty())
    return EVENT.get_form_field(fieldname);
  else
    return EVENT.get_form_field(m_formname + "[" + fieldname + "]");
}

bool wexus::turbo::has_rank(int minrank)
{
  if (minrank == 0)
    return true;

  user_session *us = dynamic_cast<user_session*>(EVENT.session);

  return us && us->rank >= minrank;
}

bool wexus::turbo::check_rank(int minrank)
{
  if (minrank > 0 && !EVENT.session) {
    EVENT.add_error("session", "You must be logged into to access this page");
    return false;
  }

  bool ret = has_rank(minrank);

  if (!ret)
    EVENT.add_error("session", "You don't have enough access rights to access this page");

  return ret;
}

//
//
// t_assert stuff
//
//

void wexus::turbo::t_assert(bool b)
{
  if (!b)
    throw t_assert_error();
}

void wexus::turbo::t_assert(bool b, const std::string &msg)
{
  if (!b) {
    EVENT.add_error("assert", msg);
    throw t_assert_error();
  }
}

void wexus::turbo::t_assert_rank(int minrank, const std::string &redirect_to_encoded_url)
{
  if (has_rank(minrank))
    return;

  redirect_to(redirect_to_encoded_url);

  throw t_assert_error();
}

void wexus::turbo::t_assert_session(void *p, std::string encodedurl)
{
  if (p)
    return;

  redirect_to(encodedurl);

  throw t_assert_error();
}

