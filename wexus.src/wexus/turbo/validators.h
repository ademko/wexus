
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

#ifndef __INCLUDED_WEXUS_TURBO_VALIDATORS_H__
#define __INCLUDED_WEXUS_TURBO_VALIDATORS_H__

#include <string>

#include <wexus/db/dbtypes.h>
#include <wexus/turbo/event.h>
#include <wexus/turbo/session.h>

namespace wexus
{
  namespace turbo
  {
    class check_form;
    
    template <class T> class session_ptr;

    void t_assert(bool b);
    void t_assert(bool b, const std::string &msg);
    void t_assert_rank(int minrank, const std::string &redirect_to_encoded_url);
    //void t_assert_noerrors(void);

    /*inline bool check_hassession(void) {
      if (!dynamic_cast<wexus::turbo::user_session*>(EVENT.session)) {
        EVENT.add_error("session", "A session is required to access this page");
        return false;
      }
      return true;
    }*/
    /*template <class T> bool check_hassession(void) {
      if (!dynamic_cast<T*>(EVENT.session)) {
        EVENT.add_error("session", "A session is required to access this page");
        return false;
      }
      return true;
    }*/

    /**
     * Checks if the current session has >=minrank.
     * Note, if minrank == 0, this is always true, regardless
     * of weither there is a session or not.
     *
     * @author Aleksander Demko
     */ 
    bool has_rank(int minrank);
    /**
     * Similar to has_rank, but ALSO sets an error on failure.
     * @author Aleksander Demko
     *
     * @author Aleksander Demko
     */ 
    bool check_rank(int minrank);
  }
}

/**
 * This is used in your handler to process forms.
 *
 * @author Aleksander Demko
 */ 
class wexus::turbo::check_form
{
  public:
    /**
     * This version simply checks all the fields .
     *
     * @author Aleksander Demko
     */ 
    check_form(void);
    /**
     * This constructor will check the form fields that where submitted
     * by the given form with the given formname.
     *
     * @author Aleksander Demko
     */ 
    check_form(const std::string &formname);

    /**
     * Check if the given field is null
     * @author Aleksander Demko
     */
    bool is_notnull(const std::string &fieldname);

    /**
     * This checks to make sure:
     *  the given field is not empty.
     *
     * On failure, false is returned an an error is added to your event stack.
     * @author Aleksander Demko
     */ 
    bool is_notempty(const std::string &fieldname);
    /**
     * This checks to make sure:
     *  is an integer
     *
     * On failure, false is returned an an error is added to your event stack.
     * @author Aleksander Demko
     */ 
    bool is_integer(const std::string &fieldname);
    /**
     * This checks to make sure:
     *  has atleast minlength chars
     *
     * On failure, false is returned an an error is added to your event stack.
     * @author Aleksander Demko
     */ 
    bool is_minlength(const std::string &fieldname, int minlength);
    /**
     * This checks to make sure:
     *  has atmost minlength chars
     *
     * On failure, false is returned an an error is added to your event stack.
     * @author Aleksander Demko
     */ 
    bool is_maxlength(const std::string &fieldname, int maxlength);
    /**
     * This checks to make sure:
     *  is a valid email address
     *
     * On failure, false is returned an an error is added to your event stack.
     * @author Aleksander Demko
     */ 
    bool is_email(const std::string &fieldname);
    /**
     * This checks to make sure:
     *  a valid username (alphanum)
     *
     * On failure, false is returned an an error is added to your event stack.
     * @author Aleksander Demko
     */ 
    bool is_username(const std::string &fieldname);

    /**
     * Check if the given field is null
     * @author Aleksander Demko
     */
    bool check_notnull(const std::string &fieldname);

    /**
     * This checks to make sure:
     *  the given field is not empty.
     *
     * On failure, false is returned an an error is added to your event stack.
     * @author Aleksander Demko
     */ 
    bool check_notempty(const std::string &fieldname);
    /**
     * This checks to make sure:
     *  is an integer
     *
     * On failure, false is returned an an error is added to your event stack.
     * @author Aleksander Demko
     */ 
    bool check_integer(const std::string &fieldname);
    /**
     * This checks to make sure:
     *  has atleast minlength chars
     *
     * On failure, false is returned an an error is added to your event stack.
     * @author Aleksander Demko
     */ 
    bool check_minlength(const std::string &fieldname, int minlength);
    /**
     * This checks to make sure:
     *  has atmost minlength chars
     *
     * On failure, false is returned an an error is added to your event stack.
     * @author Aleksander Demko
     */ 
    bool check_maxlength(const std::string &fieldname, int maxlength);
    /**
     * This checks to make sure:
     *  is a valid email address
     *
     * On failure, false is returned an an error is added to your event stack.
     * @author Aleksander Demko
     */ 
    bool check_email(const std::string &fieldname);
    /**
     * This checks to make sure:
     *  a valid username (alphanum)
     *
     * On failure, false is returned an an error is added to your event stack.
     * @author Aleksander Demko
     */ 
    bool check_username(const std::string &fieldname);

    /**
     * A general checker.
     * If b is false, the given msg is added as an error.
     * b is returned.
     * @author Aleksander Demko
     */
    bool check(bool b, const std::string &msg);

    // getter helpers

    /**
     * Retrive the given form field and put it into the local variable.
     * You should use these after youve verifired your form stuff.
     *
     * Always returns true;
     *
     * @author Aleksander Demko
     */ 
    bool get(const std::string &fieldname, wexus::db::dbint &out) const;
    /**
     * Retrive the given form field and put it into the local variable.
     * You should use these after youve verifired your form stuff.
     *
     * Always returns true;
     *
     * @author Aleksander Demko
     */ 
    bool get(const std::string &fieldname, wexus::db::dbstring &out) const;

    /**
     * A general one that returns a string.
     *
     * @author Aleksander Demko
     */ 
    std::string get(const std::string &fieldname) const { return my_get_form_field(fieldname); }

    // set and set_default methods?

    /**
     * Are there any errors in your event stack (same as EVENT.has_error()).
     *
     * all the check_ methods add to the event stack on failure.
     *
     * @author Aleksander Demko
     */ 
    static bool has_error(void);

  private:
    const std::string m_formname;

    std::string my_field(const std::string &fieldname) const;
    const std::string & my_get_form_field(const std::string &fieldname) const;
};

/**
 * This is a special pointer that automatically attempts to get the given session.
 * Manually assigning to the ptr via = will set the current session (null, ofcourse
 * clearing the current session).
 *
 * @author Aleksander Demko
 */ 
template <class T>
class wexus::turbo::session_ptr
{
  private:
    scopira::tool::count_ptr<T> m_ptr;
  public:
    /**
     * Constructor.
     * Attempts to get the current session, if any.
     *
     * @author Aleksander Demko
     */ 
    session_ptr(void) { m_ptr = dynamic_cast<T*>(EVENT.session); }

    /**
     * Assignment. Sets the current session. Null ptr clears the current session.
     *
     * @author Aleksander Demko
     */
    session_ptr & operator = (T *p)
      { m_ptr = p; EVENT.set_session(p); return *this; }

    /**
     * Gets the current object
     *
     * @author Aleksander Demko
     */
    T* get(void) const
      { return m_ptr.get(); }

    /**
     * Gets the current object, as a reference
     *
     * @author Aleksander Demko
     */
    T& ref(void) const
      { return m_ptr.ref(); }

    /**
     * Convinient -> accesor to the object
     *
     * @author Aleksander Demko
     */
    T* operator ->(void) const
      { return m_ptr.get(); }

    /**
      * Convinient * deferencer.
      *
      * @author Aleksander Demko
      */
    T & operator *(void) const
      { return m_ptr.ref(); }

    /**
     * Is the pointer null?
     *
     * @author Aleksander Demko
     */
    bool is_null(void) const { return m_ptr.is_null(); }
};

#endif

