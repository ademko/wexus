
/*
 *  Copyright (c) 2005    Aleksander B. Demko
 *
 *  All rights reserved.
 *
 *  This material is confidential and proprietary information of
 *  Aleksander B. Demko ("Confidential Information").
 *  This Confidential Information may only be used and reproduced
 *  in accordance with the terms of the license agreement.
 *
 */

#ifndef __INCLUDED_WEXUS_DB_EXCEPTION_H__
#define __INCLUDED_WEXUS_DB_EXCEPTION_H__

#include <exception>
#include <string>

#include <scopira/tool/array.h>

namespace wexus
{
  namespace turbo
  {
    class turbo_error;

    class db_error;
    class t_assert_error;
    //class t_assert_msg_error;

    //class redirect_to_error;
  }
}

class wexus::turbo::turbo_error : public std::exception
{
  public:
};

/**
 * An exception that is throw by certain methods in the db subsystem.
 *
 * @author Aleksander Demko
 */ 
class wexus::turbo::db_error : public wexus::turbo::turbo_error
{
  public:
    /// C string liternals ONLY please
    explicit db_error(const char * msg = "") throw() : m_msg(msg) { }

    virtual const char * what(void) const throw() { return m_msg; }

  private:
    const char * m_msg;
};

/**
 * An exception that is throw by certain methods in the db subsystem.
 *
 * @author Aleksander Demko
 */ 
class wexus::turbo::t_assert_error : public wexus::turbo::turbo_error
{
  public:
    virtual const char * what(void) const throw() { return 0; }
};

/**
 * An exception that is throw by certain methods in the db subsystem.
 *
 * @author Aleksander Demko
 */ 
/*class wexus::turbo::t_assert_msg_error : public wexus::turbo::turbo_error
{
  public:
    explicit t_assert_msg_error(void) throw();
    explicit t_assert_msg_error(const std::string &msg) throw();

    virtual const char * what(void) const throw() { return m_msg.c_array(); }

  private:
    scopira::tool::fixed_array<char, 128> m_msg;
};*/

/**
 * An error that when handled by the top handler, will bounce the user to the given page.
 *
 * @author Aleksander Demko
 */ 
/*class wexus::turbo::redirect_to_error : public t_assert_msg_error
{
  public:
    explicit redirect_to_error(void) throw() { }
    explicit redirect_to_error(const std::string &msg) throw() : t_assert_msg_error(msg) { }
};*/;

#endif

