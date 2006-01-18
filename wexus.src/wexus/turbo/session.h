
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

#ifndef __INCLUDED__WEXUS_TURBO_SESSION_H__
#define __INCLUDED__WEXUS_TURBO_SESSION_H__

#include <wexus/core/sessionmanager.h>
#include <wexus/db/dbtypes.h>

namespace wexus
{
  namespace turbo
  {
    class user_session;

    /**
     * Checks if the given cleartext password test
     * matches the current "crypted" password current (that is in the db).
     *
     * True if it matches.
     *
     * @author Aleksander Demko
     */ 
    bool match_password(const std::string &current, const std::string &test);

    /**
     * Taken the given cleartext password
     * and return a "crypted" one suitable for db storage.
     *
     * @author Aleksander Demko
     */ 
    std::string hash_password(const std::string &cleartext);
  }
}

/**
 * A session that contains information about a logged in user.
 *
 * This common interface can be shared among different tool kits
 * and apps to share log in sessions.
 *
 *  suggested ranks:
 *    -1 dead user
 *    0 anno users [good for posting, topic creation]
 *    1 logged in user [needed for wiki change]
 *    5 semi trusted [need for wiki create]
 *    10 moderator
 *    90 admin, including user
 *    100 master admin
 *
 * @author Aleksander Demko
 */
class wexus::turbo::user_session : public virtual wexus::core::session_i
{
  public:
    /// ctor
    user_session(void);
    /// creating ctor
    user_session(wexus::db::dbint _id, wexus::db::dbint _rank, const wexus::db::dbstring &_username,
      const wexus::db::dbstring &_realname, const wexus::db::dbstring &_email);

    /// serialization
    virtual bool load(scopira::tool::iobjflow_i& in);
    /// serialization
    virtual void save(scopira::tool::oobjflow_i& out) const;

    /// called by the turbo system
    void flush_notes(void);

  public:
    wexus::db::dbint userid, rank;
    wexus::db::dbstring username, realname, email;

    typedef std::multimap<std::string, std::string> notemap_t;
    notemap_t notes;    // rails like "flash" concept
};

#endif

