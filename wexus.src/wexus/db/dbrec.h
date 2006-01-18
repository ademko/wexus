
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

#ifndef __INCLUDED_WEXUS_DB_DBREC_HPP__
#define __INCLUDED_WEXUS_DB_DBREC_HPP__

#include <vector>

#include <wexus/rec/rec.h>

namespace wexus
{
  namespace db
  {
    class statement;//fwd

    class db_rec;
    class mem_db_rec;
  }
}

/**
 * a wexus::rec that wraps around a statement
 *
 * @author Aleksander Demko
 */ 
class wexus::db::db_rec : public wexus::rec::rec_base
{
  public:
    typedef std::vector< std::string > row_t;
  public:
    /**
     * ctor. s cannt be null and will be ref counted.
     * it should be a fresh statement that has just been executed()
     *
     * @author Aleksander Demko
     */ 
    //db_rec(statement *s, bool freshst = true);
    /// ctor, same story
    db_rec(scopira::tool::count_ptr< statement > &s, bool freshst = true);

    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const;
    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const;
    /// get a virtual iterator of all the sub recs
    virtual wexus::rec::rec_iterator get_rec_iterator(void);

    /// build a report
    virtual void report(scopira::tool::property *p) const;

  private:
    /// does the extraction work
    void init(statement &s, bool freshst);
    
  private:
    scopira::tool::count_ptr< statement > m_st;    /// the statement

    bool m_empty, m_diditer;

    size_t m_maxcolsize;
    row_t m_firstrow;

    class db_sub_iterator;
};

/**
 * this record extracs all the data from the given statement during construction
 * time and stores it in memory. after the ctor returns, this class no longer
 * uses the statement at all and retains all the result set data in memory buffers
 *
 * @author Aleksander Demko
 */ 
class wexus::db::mem_db_rec : public wexus::rec::rec_base
{
  public:
    typedef std::vector< std::string > row_t;
    typedef std::list< row_t > rowlist_t;

  public:
    /**
     * ctor. the given statement is only used during the ctor.
     *
     * @author Aleksander Demko
     */
    mem_db_rec(statement &s, bool freshst = true);
    /// ctor, same story
    mem_db_rec(scopira::tool::count_ptr< statement > &s, bool freshst = true);

    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const;
    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const;
    /// get a virtual iterator of all the sub recs
    virtual wexus::rec::rec_iterator get_rec_iterator(void);

    /// build a report
    virtual void report(scopira::tool::property *p) const;

  private:
    /// does the extraction work
    void init(statement &s, bool freshst);

  private:
    size_t m_rowcount;
    row_t m_colnames;
    rowlist_t m_dataset;

    class db_sub_iterator;
    friend class db_sub_iterator;
};

#endif

