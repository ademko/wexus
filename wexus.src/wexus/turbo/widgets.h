
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

#ifndef __INCLUDED_WEXUS_TURBO_WIDGETS_H__
#define __INCLUDED_WEXUS_TURBO_WIDGETS_H__

#include <string>

#include <wexus/db/dbtypes.h>

namespace wexus
{
  namespace turbo
  {
    class param;
    class target;
    class form;

    /**
     * Fils the form fields with the contents of the given statement row.
     * Formname may be "".
     *
     * @author Aleksander Demko
     */
    void db_to_form(wexus::db::statement &s, const std::string &formname);

    /**
     * Returns a renderable html tag that will simply link to the method in another
     * class.
     *
     * If methodname is "", "index" will be used.
     * Use option() objects for encodedoptions, if any.
     *
     * @author Aleksander Demko
     */ 
    std::string link_to(const std::string &desc, const std::string &enctargeturl);

    /// same options as link_to
    std::string button_to(const std::string &desc, const std::string &enctargeturl);

    /**
     * Mail to link. subject is optional.
     * @author Aleksander Demko
     */
    std::string mail_to(const std::string &desc, const std::string &email, const std::string &subject = "");

    /**
     * An inline image.
     * @author Aleksander Demko
     */
    std::string image_to(const std::string &enctargeturl);

    /// redirectts the user to the given destination rather than rendering output
    void redirect_to(const std::string &targeturl);

    /// stock error renderer
    void render_errors(void);
    /// stock note renderer
    void render_notes(void);
  }
}

/**
 * This reprents a name-value param that you pass to targets.
 * Basically, you make a couple of these (you can group them together
 * via +) and pass it as encodedoptions to any of the functions that take that
 *
 * @author Aleksander Demko
 */ 
class wexus::turbo::param
{
  public:
    param(const std::string &name, int value);
    param(const std::string &name, const std::string &value);

    std::string operator +(const param &rhs) const;

    operator std::string(void) const { return m_rendered_string; }

  private:
    param(const std::string &rendered_string);    // used by +

    std::string m_rendered_string;
};

/**
 * This represents a destination handler.
 *
 * if methodname is "", "index"i will be assumed.
 * encodedoptions are optional params (built via params)
 *
 * @author Aleksander Demko
 */ 
class wexus::turbo::target
{
  public:
    // CANT do implicit, because use might actually want to pass a raw string as their enctargeturl
    explicit target(const std::string &fullclassname);
    /// ctor
    target(const std::string &fullclassname, const std::string &methodname);
    /// ctor
    target(const std::string &fullclassname, const std::string &methodname, const std::string &encodedoptions);

    operator std::string(void) const { return m_rendered_string; }

  private:
    std::string m_rendered_string;
};

/**
 * This is a html form.
 *
 * @author Aleksander Demko
 */ 
class wexus::turbo::form
{
  public:
    enum {
      post_type_c,
      get_type_c,
      //upload_type_c,    // not yet implemented
    };
    enum {
      wrap_none_c = 0, // wrap won't be included if this value is specified
      wrap_off_c = 1,
      wrap_soft_c = 2,
      wrap_hard_c = 3,
    };

  public:
    /**
     * This opens a new form. formname is some unique string that you can assign it.
     * You'll need to use the same  formname when processing the form's input in your
     * handler via check_form.
     *
     * @author Aleksander Demko
     */ 
    form(const std::string &formname, const std::string &enctargeturl, short type = post_type_c);
    ~form();

    /**
     * A hidden field.
     * @author Aleksander Demko
     */ 
    std::string hidden_value(const std::string &fieldname, const std::string &defaultval = "") const;

    /**
     * A standard text field
     * @author Aleksander Demko
     */ 
    std::string text_field(const std::string &fieldname, const std::string &defaultval = "",
      int sz = 30, int maxlen = 60) const;

    /**
     * A standard text field, password version
     * @author Aleksander Demko
     */ 
    std::string password_field(const std::string &fieldname, int sz = 30, int maxlen = 60) const;

    /**
     * A standard text field, but for email addresses (might have some extra stuff in
     * the future
     * @author Aleksander Demko
     */ 
    std::string email_field(const std::string &fieldname, const std::string &defaultval = "") const
      { return text_field(fieldname, defaultval, 30, 30); }

    /**
     * A standard text area
     * @author Aleksander Demko
     */ 
    std::string text_area(const std::string &fieldname, const std::string &defaultval = "",
      int rows = 6, int cols = 60, int wraptype = wrap_none_c) const;

    /**
     * A check button.
     * @author Aleksander Demko
     */
    std::string checkbox(const std::string &fieldname, bool defaultchecked = false);

    /**
     * A radio button. You basically make a whole set of these.
     * 
     * In the future, I'll provide helper routines to make these from sets of things
     * (like queries, arrays, etc)
     *
     * @author Aleksander Demko
     */ 
    std::string radio(const std::string &fieldname, const std::string &val, bool defaultchecked = false);

    /**
     * A file upload widget
     * @author Aleksander Demko
     */ 
    std::string file_upload(const std::string &fieldname) const;

    /**
     * The submit button for the form.
     *
     * fieldname is optional, if you care for it.
     *
     * @author Aleksander Demko
     */ 
    std::string submit_button(const std::string &desc, const std::string &fieldname = "") const;

    /**
     * This "complex" widget represents a collection of options that make up
     * a drop down box.
     *
     * @author Aleksander Demko
     */
    class drop_down
    {
      public:
        /// ctor
        drop_down(form &f, const std::string &fieldname, int viewsize = 1);
        /// dtor
        ~drop_down();

        /// call this once for every option in the field
        void option(const std::string &val, const std::string &desc) const;
    };

  protected:
    std::string m_formname;

    std::string my_field(const std::string &fieldname) const;
    const std::string & my_get_form_field(const std::string &fieldname) const;
};

#endif

