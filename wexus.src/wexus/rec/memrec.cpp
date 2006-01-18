
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

#include <wexus/rec/memrec.h>

#include <scopira/tool/iterator_imp.h>
#include <scopira/tool/util.h>
#include <scopira/tool/stringflow.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus;

/*
this is the old string_iterator building code - incase itll ever be handy again
   return string_iterator(
     new tool::stl_iterator_imp< stringmap_t::iterator, string_iterator::data_type,
       tool::deref_second< stringmap_t::value_type &, string_iterator::data_type > >
       (m_strings.begin(), m_strings.end()) );
*/

rec::mem_rec::mem_rec(void)
{
  // nothin yet
}

rec::mem_rec::mem_rec(const mem_rec & src)
  : m_strings(src.m_strings), m_recs(src.m_recs)
{
}

rec::mem_rec::~mem_rec()
{
  // nothin yet
}

bool rec::mem_rec::has_string(const std::string &name) const
{
  if (name == "size" || name == "empty" || name == "report" || name == "exist")
    return true;
  else
    return m_strings.find(name) != m_strings.end();
}

void rec::mem_rec::get_string(const std::string &name, std::string &out) const
{
  if (name == "exist")
    out = m_recs.empty()?"0":"1";
  else if (name == "empty")
    out = m_recs.empty()?"1":"0";
  else if (name == "size")
    out = int_to_string(m_recs.size());
  else if (name == "report") {
    out.clear();
    stringoflow sout(&out);
    property_node rp;
    report(&rp);
    write_html_report(&rp, sout);
  } else
    out = (*m_strings.find(name)).second;
}

int rec::mem_rec::get_rec_size(void) const
{
  return m_recs.size();
}

bool rec::mem_rec::has_rec(const std::string & name) const
{
  return m_recs.count(name) > 0;
}

rec::rec_i * rec::mem_rec::get_rec(const std::string & name)
{
  return (*m_recs.find(name)).second.get();
}

rec::rec_iterator rec::mem_rec::get_rec_iterator(void)
{
  return rec_iterator(
    new stl_iterator_imp_g< recmap_t::iterator, rec_iterator::data_type,
      deref_objauto<rec_iterator::data_type,
        deref_second<recmap_t::value_type::second_type &,
          deref<recmap_t::value_type &> > > >
      (m_recs.begin(), m_recs.end()) );
}

void rec::mem_rec::report(scopira::tool::property *p) const
{
  stringmap_t::const_iterator ii, endii;
  recmap_t::const_iterator jj, endjj;
  property *r;

  if (!p->contains("rec_name"))
    p->add_end("rec_name", new property("memory record"));

  p->add_end("string", r = new property_node);
  r->add_end("string_name", new property("size"));
  r->add_end("string_desc", new property(int_to_string(m_recs.size())));
  p->add_end("string", r = new property_node);
  r->add_end("string_name", new property("empty"));
  r->add_end("string_desc", new property(m_recs.empty()?"1":"0"));
  p->add_end("string", r = new property_node);
  r->add_end("string_name", new property("exist"));
  r->add_end("string_desc", new property(m_recs.empty()?"0":"1"));

  endii = m_strings.end();
  for (ii=m_strings.begin(); ii!=endii; ++ii) {
    p->add_end("string", r = new property_node);

    r->add_end("string_name", new property((*ii).first));
    r->add_end("string_desc", new property((*ii).second));
  }
  endjj = m_recs.end();
  for (jj=m_recs.begin(); jj!=endjj; ++jj) {
    p->add_end("rec", r = new property_node);
    //recurse
    r->add_end("string_name", new property((*jj).first));
    (*jj).second->report(r);
  }
}

void rec::mem_rec::set_string(const std::string &name, const std::string &val)
{
  m_strings[name] = val;
}

void rec::mem_rec::set_rec(const std::string &name, rec_i *rec)
{
  m_recs[name] = rec;
}

void rec::mem_rec::set_rec(const std::string &name, const count_ptr<rec_i> &rec)
{
  m_recs[name] = rec;
}

void rec::mem_rec::set_flag(const std::string &name, bool set)
{
  // maybe use a different collection for flags... this will do for now though
  m_strings[name] = (set ? "1" : "0");
}

void rec::mem_rec::clear(void)
{
  m_strings.clear();
  m_recs.clear();
}

rec::msg_rec::msg_rec(const std::string &msg)
  : m_msg(msg)
{
}

rec::msg_rec::~msg_rec()
{
}

bool rec::msg_rec::has_string(const std::string &name) const
{
  return true;
}

void rec::msg_rec::get_string(const std::string &name, std::string &out) const
{
  out = m_msg;
}

void rec::msg_rec::report(scopira::tool::property *p) const
{
  property *r;

  p->add_end("rec_name", new property("message record"));

  p->add_end("string", r = new property_node);
  r->add_end("string_name", new property("msg"));
  r->add_end("string_desc", new property(m_msg));
}

