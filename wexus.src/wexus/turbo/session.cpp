
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

#include <wexus/turbo/session.h>

#include <scopira/tool/hash.h>
#include <scopira/core/register.h>
#include <wexus/turbo/event.h>

//BBtargets libwexus.so

using namespace wexus::turbo;

static scopira::tool::register_flow<user_session> r344("wexus::turbo::user_session");

bool wexus::turbo::match_password(const std::string &current, const std::string &test)
{
  return current == ("XX" + test) || current == scopira::tool::hexhash_string(test);
}

std::string wexus::turbo::hash_password(const std::string &cleartext)
{
  return scopira::tool::hexhash_string(cleartext);
}

//
//
// user_session
//
//

user_session::user_session(void)
  : userid(-1), rank(0)
{
}

user_session::user_session(wexus::db::dbint _id, wexus::db::dbint _rank, const wexus::db::dbstring &_username,
    const wexus::db::dbstring &_realname, const wexus::db::dbstring &_email)
{
  userid = _id;
  rank = _rank;
  username = _username;
  realname = _realname;
  email = _email;
}

bool user_session::load(scopira::tool::iobjflow_i& in)
{
  notes.clear();

  return
    session_i::load(in) &&

    wexus::db::read_dbint(in, userid) &&
    wexus::db::read_dbint(in, rank) &&
    username.load(in) &&
    realname.load(in) &&
    email.load(in);
}

void user_session::save(scopira::tool::oobjflow_i& out) const
{
  session_i::save(out);

  wexus::db::write_dbint(out, userid);
  wexus::db::write_dbint(out, rank);
  username.save(out);
  realname.save(out);
  email.save(out);
}

void user_session::flush_notes(void)
{
  if (notes.empty())
    return;

  notemap_t::iterator ii, endii;

  endii = notes.end();
  for (ii = notes.begin(); ii != endii; ++ii)
    EVENT.add_note(ii->first, ii->second);

  // flush em
  notes.clear();
}

