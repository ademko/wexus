
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

#ifndef __INCLUDED_WEXUS_REC_BASETOKEN_HPP__
#define __INCLUDED_WEXUS_REC_BASETOKEN_HPP__

#include <string>
#include <vector>

#include <scopira/tool/flow.h>

#include <wexus/rec/token.h>

namespace wexus
{
  namespace rec
  {
    class byte_token;
    class string_token;
    class field_token;
    class list_token;
    class foreach_token;
    class ifstmt_token;
    class if_token;
  }
}

/**
 * a token for a raw, constant array of bytes
 *
 * not a virtual decendant as its an end class
 *
 * @author Aleksander Demko
 */
class wexus::rec::byte_token : public wexus::rec::token_i
{
  public:
    /**
     * ctor. the bytetoken will call delete on the data!
     *
     * @author Aleksander Demko
     */ 
    byte_token(const scopira::tool::flow_i::byte_t *data, size_t len);
    /// dtor
    virtual ~byte_token();

    /// executes the token
    virtual void run_token(recstack_t & recstack, scopira::tool::oflow_i &output);

  private:
    const scopira::tool::flow_i::byte_t *m_data;
    size_t m_len;
};

/**
 * a token for a string
 *
 * not a virtual decendant as its an end class
 *
 * @author Aleksander Demko
 */
class wexus::rec::string_token : public wexus::rec::token_i
{
  public:
    /// ctor
    string_token(const std::string &str);
    /// dtor
    virtual ~string_token();

    /// executes the token
    virtual void run_token(recstack_t & recstack, scopira::tool::oflow_i &output);

  private:
    const std::string m_str;
};

/**
 * a token that does a field lookup
 *
 * @author Aleksander Demko
 */
class wexus::rec::field_token : public wexus::rec::token_i
{
  public:
    /// ctor
    field_token(const std::string &fieldname);
    /// dtor
    virtual ~field_token();

    /// executes the token
    virtual void run_token(recstack_t & recstack, scopira::tool::oflow_i &output);

    /**
     * gets the string value of this field
     *
     * @param outs the output string
     * @return true if something was found
     * @author Aleksander Demko
     */
    bool get_string(recstack_t & recstack, std::string & val) const;
    /**
     * gets the rec value of this field
     *
     * @return the rec, or null for not found
     * @author Aleksander Demko
     */ 
    rec_i * get_rec(recstack_t & recstack) const;

  private:
    typedef std::list< std::string > namelist_t;

    namelist_t m_fieldnames;  /// the field name, broken up
};


/**
 * a list of tokens
 *
 * not a virtual decendant as its an end class
 *
 * @author Aleksander Demko
 */
class wexus::rec::list_token : public wexus::rec::token_i
{
  public:
    /// ctor
    list_token(void);
    /// dtor
    virtual ~list_token();

    /// executes the token
    virtual void run_token(recstack_t & recstack, scopira::tool::oflow_i &output);

    /// adds the given token to the list. it will be refcounted/managed by this class
    void add_token(token_i *tok);

  private:
    typedef std::list< scopira::tool::count_ptr< token_i > > list_t;

    list_t m_list;
};

/**
 * iterates through all the tokens
 *
 * @author Aleksander Demko
 */
class wexus::rec::foreach_token : public wexus::rec::list_token
{
  public:
    /// ctor
    foreach_token(const std::string &fieldname);
    /// dtor
    virtual ~foreach_token();

    /// executes the token
    virtual void run_token(recstack_t & recstack, scopira::tool::oflow_i &output);

  private:
    field_token m_ftok;
};

/**
 * if statement-list iteration token
 *
 * @author Andrew Kaspick
 */
class wexus::rec::ifstmt_token : public wexus::rec::list_token
{
  public:
    /// ctor... i dont like to inline ctors with complex start ups
    ifstmt_token(void);

    /// executes the token
    virtual void run_token(recstack_t& recstack, scopira::tool::oflow_i& output);

    /// adds an if token to the if statement
    void add_if_token(if_token* iftok);

  private:
    typedef std::vector< scopira::tool::count_ptr<if_token> > iftokens_t;
    iftokens_t  m_iftokens; /// list of if branch tokens in the if statement
};

/**
 * if branch token-list iteration token
 * if branch tokens can be any of the following...
 * if[not], elseif[not], else
 *
 * @author Andrew Kaspick
 */
class wexus::rec::if_token : public wexus::rec::list_token
{
  public:
    /**
     * ctor
     *
     * @param fieldname field of if token to test against
     * @param is_if is the token an 'if' variant opposed to an 'ifnot' variant
     * @author Andrew Kaspick
     */
    if_token(const std::string& fieldname, bool is_if);

    /// executes the token
    virtual void run_token(recstack_t & recstack, scopira::tool::oflow_i &output);

    /// is the flag an if or ifnot variant
    bool is_if(void) const { return m_if; }
    /// the field token
    const field_token* field(void) const { return m_ftok.get(); }

  protected:
    scopira::tool::count_ptr<field_token> m_ftok;
    bool m_if;
};

#endif

