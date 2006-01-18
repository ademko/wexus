
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

#include <wexus/db/dbrec.h>

#include <scopira/tool/util.h>

#include <wexus/db/statement.h>

#include <algorithm>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus;

class db::db_rec::db_sub_iterator : public scopira::tool::iterator_imp_g<rec::rec_i*>,
  rec::rec_base
{
  public:
    typedef db::db_rec::row_t row_t;
  public:
    /// ctor
    db_sub_iterator(db_rec *parent);

    virtual rec::rec_i* current(void);
    virtual bool valid(void) const;
    virtual void next(void);

    virtual bool has_string(const std::string &name) const;
    virtual void get_string(const std::string &name, std::string &out) const;

  private:
    count_ptr< db_rec > m_parent;
    bool m_ok, m_first;
    row_t m_currow;
};

//db::db_rec::db_rec(statement *s, bool freshst)
//  : m_st(s)
//{
//  assert(s);
//  init(*s, freshst);
//}

db::db_rec::db_rec(scopira::tool::count_ptr< statement > &s, bool freshst)
  : m_st(s)
{
  assert(s.get());
  init(s.ref(), freshst);
}

bool db::db_rec::has_string(const std::string &name) const
{
  short dummy;
  return name == "empty" || name == "exist" || m_st->find_colnum(name, dummy);
}

void db::db_rec::get_string(const std::string &name, std::string &out) const
{
  if (name == "exist") {
    out = m_empty?"0":"1";
    return;
  }
  if (name == "empty") {
    out = m_empty?"1":"0";
    return;
  }
  short id;
  if (!m_st->find_colnum(name, id)) {
    assert(false);
    return;
  }

  if (m_empty)
    out.clear(); //no rows!
  else
    out = m_firstrow[id];
}


rec::rec_iterator db::db_rec::get_rec_iterator(void)
{
  if (m_empty || m_diditer)
    return rec::rec_iterator(new null_iterator_imp_g<rec::rec_iterator::data_type>);
  else {
    m_diditer = true; //dont try this again now, you hear!
    return rec::rec_iterator(new db_sub_iterator(this));
  }
}

void db::db_rec::report(scopira::tool::property *p) const
{
  property *r, *subr, *subname;

  p->add_end("rec_name", new property("database query"));

  p->add_end("string", r = new property_node);
  r->add_end("string_name", new property("empty"));
  r->add_end("string_desc", new property(m_empty?"1":"0"));

  p->add_end("string", r = new property_node);
  r->add_end("string_name", new property("exist"));
  r->add_end("string_desc", new property(m_empty?"0":"1"));

  p->add_end("rec", subr = new property_node);
  subr->add_end("rec_name", new property("result set"));

  const std::string dbfield("field");
  typedef db::statement::fieldnames_t T;
  const T &f = m_st->get_fieldnames();
  T::const_iterator ii, endii;
  endii = f.end();
  for (ii=f.begin(); ii!=endii; ++ii) {
    subr->add_end("string", r = new property_node);

    r->add_end("string_name", subname = new property(*ii));
    r->add_end("string_desc", new property(dbfield));

    p->add_end("string", r = new property_node); // yes, the parent has one too
    r->add_end("string_name", subname); // we can reuse, thanks to ref counting!
    if (!m_empty)
      r->add_end("string_desc", new property(* (m_firstrow.begin() + (ii - f.begin()))));
  }
}

void db::db_rec::init(statement &s, bool freshst)
{
  m_diditer = false;

  if (freshst)
    m_empty = !m_st->fetch_next();
  else
    m_empty = false;

  /// fill the first row in
  if (!m_empty) {
    //size up the row
    const db::statement::fieldsizes_t &sz = s.get_fieldsizes();
    m_firstrow.resize(sz.size());
    assert(m_firstrow.size()>0);

    // get max element size
    m_maxcolsize = *std::max_element(sz.begin(), sz.end());
    assert(m_maxcolsize>0);
    db::dbstring buffy(m_maxcolsize);

    // fill in the row
    for (size_t i=0; i<m_firstrow.size(); ++i) {
      s.get_string(i, buffy);
      buffy.str(m_firstrow[i]);
    }
  }
}

db::db_rec::db_sub_iterator::db_sub_iterator(db_rec *parent)
  : m_parent(parent), m_ok(true), m_first(true)  //first is definaly ok
{
  assert(parent);
}

rec::rec_i* db::db_rec::db_sub_iterator::current(void)
{
  return this;
}

bool db::db_rec::db_sub_iterator::valid(void) const
{
  return m_ok;
}

void db::db_rec::db_sub_iterator::next(void)
{
  m_ok = m_parent->m_st->fetch_next();
  if (m_first) {
    m_first = false;
    m_currow.resize(m_parent->m_firstrow.size());
  }

  // fill in the next row
  db::dbstring buffy(m_parent->m_maxcolsize);
  // fill in the row
  for (size_t i=0; i<m_currow.size(); ++i) {
    m_parent->m_st->get_string(i, buffy);
    buffy.str(m_currow[i]);
  }
}

bool db::db_rec::db_sub_iterator::has_string(const std::string &name) const
{
  short id;

  assert(m_ok);
  return m_parent->m_st->find_colnum(name, id);
}

void db::db_rec::db_sub_iterator::get_string(const std::string &name, std::string &out) const
{
  assert(m_ok);

  if (m_first)
    m_parent->get_string(name, out);  // redirect request to parent, who has first row already
  else {
    short id;
#ifndef NDEBUG
    bool b=
#endif
    m_parent->m_st->find_colnum(name, id);
    assert(b);
    assert(id < m_currow.size());
    out = m_currow[id];
  }
}

//
// mem_db_rec (iterator)
//

class db::mem_db_rec::db_sub_iterator : public iterator_imp_g<rec::rec_i*>,
  rec::rec_base
{
  public:
    typedef db::mem_db_rec::row_t row_t;
    typedef db::mem_db_rec::rowlist_t rowlist_t;
  public:
    /// ctor
    db_sub_iterator(mem_db_rec &parent);

    virtual rec::rec_i* current(void);
    virtual bool valid(void) const;
    virtual void next(void);

    virtual bool has_string(const std::string &name) const;
    virtual void get_string(const std::string &name, std::string &out) const;

  private:
    const row_t &m_colnames;
    rowlist_t::iterator m_ii, m_endii;
};


//
// mem_db_rec
//

db::mem_db_rec::mem_db_rec(statement &s, bool freshst)
  : m_colnames(s.get_fieldnames())
{
  init(s, freshst);
}

/// ctor, same story
db::mem_db_rec::mem_db_rec(scopira::tool::count_ptr< statement > &s, bool freshst)
  : m_colnames(s->get_fieldnames())
{
  init(s.ref(), freshst);
}

bool db::mem_db_rec::has_string(const std::string &name) const
{
  return name == "empty" || name == "exist" || name == "size" ||
    m_colnames.end() != std::find(m_colnames.begin(), m_colnames.end(), name);
}

void db::mem_db_rec::get_string(const std::string &name, std::string &out) const
{
  if (name == "exist") {
    out = m_rowcount != 0 ? "1" : "0";
    return;
  }
  if (name == "empty") {
    out = m_rowcount == 0 ? "1" : "0";
    return;
  }
  if (name == "size") {
    size_t_to_string(m_rowcount, out);
    return;
  }
  row_t::const_iterator ii = std::find(m_colnames.begin(), m_colnames.end(), name);

  assert(ii != m_colnames.end());

  if (m_rowcount == 0)
    out.clear(); //no rows!
  else
    out = * ((*m_dataset.begin()).begin() + (ii - m_colnames.begin()));
}

wexus::rec::rec_iterator db::mem_db_rec::get_rec_iterator(void)
{
  if (m_rowcount == 0)
    return rec::rec_iterator(new null_iterator_imp_g<rec::rec_iterator::data_type>);
  else
    return rec::rec_iterator(new db_sub_iterator(*this));
}

void db::mem_db_rec::report(scopira::tool::property *p) const
{
  property *r, *subr, *subname;

  p->add_end("rec_name", new property("buffered query"));

  p->add_end("string", r = new property_node);
  r->add_end("string_name", new property("empty"));
  r->add_end("string_desc", new property(m_rowcount==0?"1":"0"));

  p->add_end("string", r = new property_node);
  r->add_end("string_name", new property("exist"));
  r->add_end("string_desc", new property(m_rowcount!=0?"1":"0"));

  p->add_end("string", r = new property_node);
  r->add_end("string_name", new property("size"));
  r->add_end("string_desc", new property(int_to_string(m_rowcount)));

  p->add_end("rec", subr = new property_node);
  subr->add_end("rec_name", new property("result set"));

  const std::string dbfield("field");
  row_t::const_iterator ii, endii;
  endii = m_colnames.end();
  for (ii=m_colnames.begin(); ii!=endii; ++ii) {
    subr->add_end("string", r = new property_node);

    r->add_end("string_name", subname = new property(*ii));
    r->add_end("string_desc", new property(dbfield));

    p->add_end("string", r = new property_node); // yes, the parent has one too
    r->add_end("string_name", subname); // we can reuse, thanks to ref counting!
    if (!m_dataset.empty())
      r->add_end("string_desc", new property(* ((*m_dataset.begin()).begin() + (ii - m_colnames.begin()))));
  }
}

void db::mem_db_rec::init(statement &s, bool freshst)
{
  size_t i, w, maxcolsize;
  const db::statement::fieldsizes_t &sz = s.get_fieldsizes();

  w = m_colnames.size();
  assert(w>0);

  maxcolsize = *std::max_element(sz.begin(), sz.end());
  assert(maxcolsize>0);

  db::dbstring buffy(maxcolsize);

  m_rowcount = 0;
  if (freshst && !s.fetch_next())
    return;
  do {
    m_rowcount++;
    m_dataset.push_back(row_t());  // push a new row
    row_t &r = *m_dataset.rbegin();

    r.resize(w);
    for (i=0; i<w; ++i) {
      s.get_string(i, buffy);
      buffy.str(r[i]);
    }
  } while (s.fetch_next());
}

db::mem_db_rec::db_sub_iterator::db_sub_iterator(mem_db_rec &parent)
  : m_colnames(parent.m_colnames), m_ii(parent.m_dataset.begin()), m_endii(parent.m_dataset.end())
{
}

rec::rec_i* db::mem_db_rec::db_sub_iterator::current(void)
{
  return this;
}

bool db::mem_db_rec::db_sub_iterator::valid(void) const
{
  return m_ii != m_endii;
}

void db::mem_db_rec::db_sub_iterator::next(void)
{
  ++m_ii;
}

bool db::mem_db_rec::db_sub_iterator::has_string(const std::string &name) const
{
  return m_colnames.end() != std::find(m_colnames.begin(), m_colnames.end(), name);
}

void db::mem_db_rec::db_sub_iterator::get_string(const std::string &name, std::string &out) const
{
  assert(valid());
  row_t::const_iterator ii = std::find(m_colnames.begin(), m_colnames.end(), name);

  assert(ii != m_colnames.end());

  out = * ((*m_ii).begin() + (ii - m_colnames.begin()));
}

