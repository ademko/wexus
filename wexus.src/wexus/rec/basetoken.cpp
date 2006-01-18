
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

#include <wexus/rec/basetoken.h>

#include <scopira/tool/util.h>

#include <wexus/rec/rec.h>

//BBtargets libwexus.so

using namespace wexus;

rec::byte_token::byte_token(const scopira::tool::flow_i::byte_t* data, size_t len)
  : m_data(data), m_len(len)
{
  assert(m_data);
  assert(m_len > 0);
}

rec::byte_token::~byte_token()
{
  delete[] m_data;
}

void rec::byte_token::run_token(recstack_t& recstack, scopira::tool::oflow_i& output)
{
  output.write(m_data, m_len);
}

rec::string_token::string_token(const std::string& str)
  : m_str(str)
{
}

rec::string_token::~string_token()
{
}

void rec::string_token::run_token(recstack_t& recstack, scopira::tool::oflow_i& output)
{
  output << m_str;
}

rec::field_token::field_token(const std::string& fieldname)
{
  // break up fieldname by . into the list
  scopira::tool::string_tokenize_word(fieldname, m_fieldnames, ".");
  assert(m_fieldnames.size() > 0);
}

rec::field_token::~field_token()
{
  // nothing
}


void rec::field_token::run_token(recstack_t& recstack, scopira::tool::oflow_i& output)
{
  std::string s;

  if (get_string(recstack, s))
    output << s;
}

bool rec::field_token::get_string(recstack_t& recstack, std::string& val) const
{
  recstack_t::const_iterator jj;
  namelist_t::const_iterator ii;
  namelist_t::size_type didc;
  rec::rec_i *rc;
  const std::string * goonlast;
  bool goon;   // go on

  goonlast = 0;
  // iterate over the rec stack
  for (jj=recstack.begin(); jj != recstack.end(); ++jj) {
    // iterate over the names
    rc = *jj;
    assert(rc);
    didc = 0;
    goon = true;
    for (ii=m_fieldnames.begin(); goon && (ii != m_fieldnames.end()); ++ii) {
      // drill down and try to find the fully qualified token name
      if (rc->has_rec(*ii)) {
        rc = rc->get_rec(*ii);
        didc++;
      } else {
        goon = false;
        goonlast = & (*ii);
      }
    }//for

    // matches the hole fieldname, then tack a "string" onto it and try it
    if (didc == m_fieldnames.size()) {
      if (rc->has_string("string")) {
        rc->get_string("string", val);
        return true;
      }
      // this rec doesnt have a "string" prop - continue on with the search
    } else if (didc == (m_fieldnames.size() - 1)) {
        // we did one less than the required, try it as the string prop
        // ii should currently be pointing at the last-string in question

        assert(!goon);
        assert(goonlast);
        if (rc->has_string(*goonlast)) {
          rc->get_string(*goonlast, val);
          return true;
        }
        // else fall through and continue the hunt
      }
  }//for

  return false;
}

rec::rec_i * rec::field_token::get_rec(recstack_t& recstack) const
{
  recstack_t::const_iterator jj;
  namelist_t::const_iterator ii;
  rec::rec_i *rc;

  // iterate over the rec stack
  for (jj=recstack.begin(); jj != recstack.end(); ++jj) {
    // iterate over the names
    rc = *jj;
    assert(rc);
    for (ii=m_fieldnames.begin(); rc && (ii != m_fieldnames.end()); ++ii) {
      // drill down and try to find the fully qualified token name
      if (rc->has_rec(*ii))
        rc = rc->get_rec(*ii);
      else
        rc = 0;
    }//for

    if (rc)
      return rc;
  }//for

  return 0; // not found
}

rec::list_token::list_token(void)
{
}

rec::list_token::~list_token()
{
}

void rec::list_token::run_token(recstack_t& recstack, scopira::tool::oflow_i& output)
{
  list_t::iterator ii;

  assert(is_alive_object());
  for (ii=m_list.begin(); ii != m_list.end(); ++ii)
    (*ii)->run_token(recstack, output);
}

void rec::list_token::add_token(token_i* tok)
{
  m_list.push_back(tok);
}

rec::foreach_token::foreach_token(const std::string& fieldname)
  : m_ftok(fieldname)
{
}

rec::foreach_token::~foreach_token()
{
}

void rec::foreach_token::run_token(recstack_t& recstack, scopira::tool::oflow_i& output)
{
  // scan the rec/scope tree until we find somthing
  rec::rec_i* cur = m_ftok.get_rec(recstack);

  if (!cur)
    return;
 
  // loop over this records sub recs
  // push the found rec to the scope-stack
  recstack.push_front(cur);

  // loop through all the sub recs of cur
  for (rec::rec_iterator ii=cur->get_rec_iterator(); ii.valid(); ++ii) {
    // push this sub rec to the scope stack
    assert(*ii);
    recstack.push_front(*ii);
    // run all my tokens on this new recstack
    list_token::run_token(recstack, output);
    // pop the stack
    recstack.pop_front();
  }

  // pop the rec i added to the stack
  recstack.pop_front();
}

//**************************
// if statement token
//**************************
rec::ifstmt_token::ifstmt_token(void)
{
}

void rec::ifstmt_token::run_token(recstack_t& recstack, scopira::tool::oflow_i& output)
{
  std::string val;
  bool foundit;

  // loop through all if statment tokens and running the *one* that is true
  for (iftokens_t::iterator it=m_iftokens.begin(); it!=m_iftokens.end(); it++)
  {
    bool isif = (*it)->is_if();
    const field_token* field = (*it)->field();

    if (field)
      foundit = field->get_string(recstack, val);
    else
    {
      // else tokens have no field and are always run if all previous
      // tokens failed to run.  set found to true and provide a valid value.
      foundit = true;
      val = "1";
    }

    // run the token if it's found and has a valid value and the token is an if
    // or
    // run the token if it's not found and the the token is an ifnot
    if ( (foundit && !val.empty() && val!="0") == isif )
    {
      (*it)->run_token(recstack, output);
      break; // only run one token in an if statement
    }
  }
}

void rec::ifstmt_token::add_if_token(if_token* iftok)
{
  // this routine isnt trivial and shouldnt be inlined
  m_iftokens.push_back(iftok);
}

//**************************
// if token
//**************************
rec::if_token::if_token(const std::string& fieldname, bool isif)
  : m_if(isif)
{
  // create a field token if the fieldname has a value
  // else tokens have no field
  if (!fieldname.empty())
    m_ftok = new field_token(fieldname);
}

void rec::if_token::run_token(recstack_t& recstack, scopira::tool::oflow_i& output)
{
  list_token::run_token(recstack, output);
}

