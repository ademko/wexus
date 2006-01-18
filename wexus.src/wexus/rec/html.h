
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

#ifndef __INCLUDED_WEXUS_REC_HTML_HPP__
#define __INCLUDED_WEXUS_REC_HTML_HPP__

#include <list>

#include <wexus/rec/memrec.h>

namespace wexus
{
  namespace rec
  {
    class form_hrec;
    class button_hrec;
    class checkbox_hrec;
    class fileupload_hrec;
    class hidden_hrec;
    class image_hrec;
    class link_hrec;
    class list_hrec;
    class radio_hrec;
    class textfield_hrec;
    class textarea_hrec;
  }
}

/**
 * a form or container of other widgets
 *
 * @author Aleksander Demko
 */ 
class wexus::rec::form_hrec : public wexus::rec::mem_rec
{
  public:
    // methods
    enum {
      none_c = -1,
      get_c = 0,
      post_c = 1,
      fileupload_c = 2  //file/upload
    };

  public:
    /**
     * ctor
     *
     * Action-less form.  Used to wrap form controls that perform
     * the action instead of the form itself (needed with javascript etc.).
     *
     * @param name the form name
     * @author Andrew Kaspick
     */ 
    form_hrec(const std::string& name);
    /**
     * ctor
     *
     * @param name the form name
     * @param action the action url
     * @param method the method of the action (get/post/fileupload)
     * @author Aleksander Demko
     */ 
    form_hrec(const std::string& name, const std::string &action, int method=get_c);

    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const;
    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const;

    /// build a report
    virtual void report(scopira::tool::property *p) const;

    /// add a quick hidden tag
    void add_hidden(const std::string &name, const std::string &value);

  private:

    std::string m_name; // form name
    int m_method;       // form method (only get/post)
    bool m_upload;      // file upload?
    std::string m_actionurl; //actionurl
    std::string m_hiddens;
};

/**
 * a html button
 *
 * @author Aleksander Demko
 */
class wexus::rec::button_hrec : public wexus::rec::rec_base
{
  public:
    // button types
    enum {
      submit_c = 0,
      reset_c = 1
    };

  public:
    /**
     * ctor
     *
     * @param typ the type
     * @param desc the visable description of the button (0 length == default)
     * @author Aleksander Demko
     */
    button_hrec(int type = submit_c, const std::string &desc = "");

    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const;
    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const;

    /// build a report
    virtual void report(scopira::tool::property *p) const;

  private:
    int m_type;         /// button type
    std::string m_desc; /// the description
};

/**
 * a checkbox widget
 *
 * @author Aleksander Demko
 */
class wexus::rec::checkbox_hrec : public wexus::rec::rec_base
{
  public:
    /**
     * ctor
     *
     * @param name the name of the widget
     * @param val the value
     * @param desc the description
     * @param checked is checked?
     * @author Aleksander Demko
     */
    checkbox_hrec(const std::string &name, const std::string &val,
      bool checked = false, const std::string &desc = "no desc");

    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const;
    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const;

    /// build a report
    virtual void report(scopira::tool::property *p) const;

  private:
    std::string m_name;
    std::string m_val;
    std::string m_desc;
    bool m_checked;
};

/**
 * file upload widget
 *
 * @author Aleksander Demko
 */
class wexus::rec::fileupload_hrec : public wexus::rec::rec_base
{
  public:
    /**
     * ctor
     *
     * @param name widget name
     * @author Aleksander Demko
     */ 
    fileupload_hrec(const std::string &name);

    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const;
    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const;

    /// build a report
    virtual void report(scopira::tool::property *p) const;

  private:
    std::string m_name;
};

/**
 * hidden widget. You can use form.add_hidden instead, its easier and slicker
 *
 * @author Aleksander Demko
 */
class wexus::rec::hidden_hrec : public wexus::rec::rec_base
{
  public:
    /**
     * ctor
     *
     * @param name the name of the widget
     * @param val the value
     * @param desc the description
     * @param checked is checked?
     * @author Aleksander Demko
     */
    hidden_hrec(const std::string &name, const std::string &val);

    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const;
    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const;

    /// build a report
    virtual void report(scopira::tool::property *p) const;

  private:
    std::string m_name;
    std::string m_val;
};

/**
 * image widget
 *
 * @author Aleksander Demko
 */
class wexus::rec::image_hrec : public wexus::rec::rec_base
{
  public:
    /**
     * ctor
     *
     * @param url url of the image
     * @author Aleksander Demko
     */ 
    image_hrec(const std::string &url);

    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const;
    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const;

    /// build a report
    virtual void report(scopira::tool::property *p) const;

  private:
    std::string m_url;
};

/**
 * link widget
 *
 * @author Aleksander Demko
 */
class wexus::rec::link_hrec : public wexus::rec::rec_base
{
  public:
    /**
     * ctor
     *
     * @param url url of the link
     * @param desc the description
     * @author Aleksander Demko
     */ 
    link_hrec(const std::string &url, const std::string &desc);

    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const;
    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const;

    /// build a report
    virtual void report(scopira::tool::property *p) const;

  private:
    std::string m_url;
    std::string m_desc;
};

/**
 * list widget
 *
 * @author Aleksander Demko
 */
class wexus::rec::list_hrec : public wexus::rec::rec_base
{
  public:
    /**
     * ctor
     *
     * @param name the name of this widget
     * @param sz the size of the widget. 1 == drop down box
     * @param multi can select more than one item at a time?
     * @author Aleksander Demko
     */ 
    list_hrec(const std::string &name, int sz = 1, bool multi = false);

    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const;
    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const;

    /// build a report
    virtual void report(scopira::tool::property *p) const;

    /// adds an option to the list
    void add_option(const std::string &val, const std::string &desc, bool selected = false);
    /// adds a set of options
    void add_option(rec_iterator &ii, const std::string &varfield, const std::string descfield);
    /// set the option with the given value to be selected
    void set_selected(const std::string &val);
    /// get the options count
    int get_options_size(void) const { return m_size; }

  private:
    std::string m_name;
    int m_size;
    bool m_multi;

    struct option_t {
      std::string value;
      std::string desc;
      bool selected;
      option_t(void) : selected(false) { }
      option_t(const std::string &_v, const std::string &_d, bool _sel = false)
        : value(_v), desc(_d), selected(_sel) { }
    };

    typedef std::list<option_t> optlist_t;

    optlist_t m_options;      /// current list of options
};

/**
 * a radio button widget
 *
 * @author Aleksander Demko
 */
class wexus::rec::radio_hrec : public wexus::rec::rec_base
{
  public:
    /**
     * ctor
     *
     * @param name the name of the widget
     * @param val the value
     * @param desc the description
     * @param checked is checked?
     * @author Aleksander Demko
     */
    radio_hrec(const std::string &name, const std::string &val,
      bool checked = false, const std::string &desc = "no desc");

    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const;
    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const;

    /// build a report
    virtual void report(scopira::tool::property *p) const;

  private:
    std::string m_name;
    std::string m_val;
    std::string m_desc;
    bool m_checked;
};

/**
 * text field
 *
 * @author Aleksander Demko
 */
class wexus::rec::textfield_hrec : public wexus::rec::rec_base
{
  public:
    enum {
      normal_c = 0,
      password_c = 1
    };

  public:
    /**
     * ctor
     *
     * @param name the name of the widget
     * @param def the default value
     * @param sz the visual size
     * @param maxlen the logical max of the string
     * @param type type
     * @author Aleksander Demko
     */
    textfield_hrec(const std::string &name, const std::string &def = "",
      int sz = 30, int maxlen = 60, int type = normal_c);

    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const;
    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const;

    /// build a report
    virtual void report(scopira::tool::property *p) const;

  private:
    std::string m_name;       // name of widget
    std::string m_def;        // default string
    int m_size;          // size
    int m_maxlen;        // maxlen
    int m_type;          // the button type
};

/**
 * textarea widget
 *
 * @author Aleksander Demko
 */
class wexus::rec::textarea_hrec : public wexus::rec::rec_base
{
  public:
    // wrap type
    // deprecated, actually never std, but included due to it's wide use
    enum {
      none_c = 0, // wrap won't be included if this value is specified
      off_c = 1,
      soft_c = 2,
      hard_c = 3
    };

  public:
    /**
     * ctor
     *
     * @param name widget name
     * @param def value
     * @param rows number of rows
     * @param cols number of cols
     * @param wrap wrap type (deprecated)
     */
    textarea_hrec(const std::string &name, const std::string &def = "",
      int rows = 6, int cols = 60, int wrap = none_c);

    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const;
    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const;

    /// build a report
    virtual void report(scopira::tool::property *p) const;

  private:
    std::string m_name;  // name of widget
    std::string m_def;   // default string
    int m_rows;          // rows
    int m_cols;          // cols
    int m_wrap;          // the wrap type
};

#endif

