
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

#ifndef __INCLUDED_WEXUS_REC_REC_HPP__
#define __INCLUDED_WEXUS_REC_REC_HPP__

#include <string>

#include <scopira/tool/object.h>
#include <scopira/tool/iterator.h>
#include <scopira/tool/prop.h>

#include <wexus/rec/basetoken.h>

namespace scopira
{
  namespace tool
  {
    class iflow_i;
    class oflow_i;
    class property;
  }
}

namespace wexus
{
  /**
   * This namespace contains the various base classes, tools, parsers and
   * utility classes for record templating.
   * @author Aleksander Demko
   */ 
  namespace rec
  {
    class rec_i;
    class rec_base;

    // forward
    class token_i;
    class list_token;

    /**
     * merge the given rec tree and token tree into the output stream
     *
     * @author Aleksander Demko
     */ 
    void merge_rec_and_token(rec_i &root, token_i &tok, scopira::tool::oflow_i &out);

    /**
     * merge a rec tree, a string "file" to the given output stream
     *
     * @author Aleksander Demko
     */
    void merge_rec_and_string(rec_i &root, const std::string &str, scopira::tool::oflow_i &out);

    /**
     * writes the give report as an html to the given flow
     *
     * @author Aleksander Demko
     */ 
    void write_html_report(scopira::tool::property *rp, scopira::tool::oflow_i &out);

    class tokenizer_i;
    class text_tokenizer;
    // future versions include binary type ones?

    // this is important, no need to restrict it to rec_i
    /// rec iterator
    typedef scopira::tool::iterator_g<rec_i*> rec_iterator;
  }
}

/**
 * tokenizers base class
 *
 * @author Aleksander Demko
 */ 
class wexus::rec::tokenizer_i : public virtual scopira::tool::object
{
  public:
    /**
     * tokenizer operation. use must delete the returned token
     *
     * @author Aleksander Demko
     */
    virtual token_i * operator ()(scopira::tool::iflow_i &in) = 0;
};

/**
 * a text tokenizer
 *
 * @author Aleksander Demko
 */ 
class wexus::rec::text_tokenizer : public wexus::rec::tokenizer_i
{
  public:
    // in the future, add a ctor that will take field idents

    /**
     * text tokenizer. takes a text file of salt tokens and builds a token
     * tree
     * 
     * @param in input stream
     * @return a NEWly alocated token that should be deleted
     * @author Aleksander Demko
     */
    virtual token_i* operator()(scopira::tool::iflow_i &in);

  private:
    enum { none_c, end_c, else_c, elseif_c };
    int text_tokenizer_level(scopira::tool::iflow_i& in, wexus::rec::list_token& listt, ifstmt_token* ifstmt = 0);
};
/**
 * base interface for all template records
 *
 * @author Aleksander Demko
 */ 
class wexus::rec::rec_i : public virtual scopira::tool::object
{
  public:

    /// string iterator
    //typedef scopira::tool::iterator<const std::string &> string_iterator;
    //string iteration is not needed. and until it is, it wont be included.


    /**
     * get the number of string properties in this rec
     *
     * @author Aleksander Demko
     */ 
    // removed.
    //virtual int get_string_size(void) = 0;

    /**
     * does the rec have this particular string prop?
     *
     * @author Aleksander Demko
     */
    virtual bool has_string(const std::string &name) const = 0;

    /**
     * get a string property by its key name
     *
     * @author Aleksander Demko
     */ 
    virtual void get_string(const std::string &name, std::string &out) const = 0;

    /**
     * get a virtual iterator of all the strings
     *
     * @author Aleksander Demko
     */ 
    // removed.
    //virtual string_iterator get_string_iterator(void) = 0;

    /**
     * get the number of sub recs in this rec
     *
     * @author Aleksander Demko
     */ 
    virtual int get_rec_size(void) const = 0;

    /**
     * does this rec have this particular sub rec?
     *
     * @author Aleksander Demko
     */
    virtual bool has_rec(const std::string & name) const = 0;

    /**
     * get a sub rec by name, might be null
     *
     * make this const?
     *
     * @author Aleksander Demko
     */
    virtual rec_i * get_rec(const std::string & name) = 0;

    /**
     * get a virtual iterator of all the sub recs
     *
     * make this const?
     *
     * @author Aleksander Demko
     */ 
    virtual rec_iterator get_rec_iterator(void) = 0;

    /**
     * this "describes" the record to the given report type.
     * this version just does some generic methods, decendants should overide
     * and implement their own.<p>
     * The final report should be of the form:
     * { rec_name "name" string { string_name "x" string_desc "y" } string
     * {..} rec { [string_name] RECURSE } }
     *
     * @author Aleksander Demko
     */ 
    virtual void report(scopira::tool::property *p) const;

  protected:
};

/**
 * a simple "base"/helper for rec implementaitons
 * simply returns 0/null for many things
 *
 * @author Aleksander Demko
 */ 
class wexus::rec::rec_base : public virtual wexus::rec::rec_i
{
  public:
    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const { return false; }

    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const { }

    /// get the number of sub recs in this rec
    virtual int get_rec_size(void) const { return 0; }

    /// does this rec have this particular sub rec?
    virtual bool has_rec(const std::string & name) const { return false; }

    /// get a sub rec by name, might be null
    virtual rec_i * get_rec(const std::string & name) { return 0; }

    /// get a virtual iterator of all the sub recs
    virtual rec_iterator get_rec_iterator(void);
};

/// output for rec_i
scopira::tool::oflow_i& operator<<(scopira::tool::oflow_i& o, wexus::rec::rec_i &val);

#endif

