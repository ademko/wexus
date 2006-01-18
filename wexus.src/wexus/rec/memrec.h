
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

#ifndef __INCLUDED_WEXUS_REC_MEMREC_HPP__
#define __INCLUDED_WEXUS_REC_MEMREC_HPP__

#include <map>

#include <wexus/rec/rec.h>

namespace wexus
{
  namespace rec
  {
    class mem_rec;
    class msg_rec;
  }
}

/**
 * a simple, setable in memory storage rec. should this be
 * serializable?
 *
 * @author Aleksander Demko
 */ 
class wexus::rec::mem_rec : public virtual wexus::rec::rec_i
{
  public:
    /// ctor
    mem_rec(void);
    /// copy ctor
    mem_rec(const mem_rec & src);
    /// dtor
    virtual ~mem_rec();

    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const;

    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const;

    /// get the number of sub recs in this rec
    virtual int get_rec_size(void) const;

    /// does this rec have this particular sub rec?
    virtual bool has_rec(const std::string & name) const;

    /// get a sub rec by name, might be null
    virtual rec_i * get_rec(const std::string & name);

    /// get a virtual iterator of all the sub recs
    virtual rec_iterator get_rec_iterator(void);

    /// build a report
    virtual void report(scopira::tool::property *p) const;

    /// sets a string
    void set_string(const std::string &name, const std::string &val);
    /// sets a record, will be ref counted.
    void set_rec(const std::string &name, rec_i *rec);
    /// sets a record, will be ref counted, alternate signature
    void set_rec(const std::string &name, const scopira::tool::count_ptr<rec_i> &rec);

    /// sets a flag
    void set_flag(const std::string& name, bool set=true);

    /// clears everyting
    void clear(void);

  private:

    typedef std::map< std::string, std::string > stringmap_t;
    typedef std::map< std::string, scopira::tool::count_ptr< rec_i > > recmap_t;

    /// the strings
    stringmap_t m_strings;
    /// the recs
    recmap_t m_recs;
};

/**
 * a rec that always returns the same msg string
 *
 * @author Aleksander Demko
 */
class wexus::rec::msg_rec : public wexus::rec::rec_base
{
  public:
    /// ctor
    msg_rec(const std::string &msg);
    /// dtor
    virtual ~msg_rec();

    /// does the rec have this particular string prop?
    virtual bool has_string(const std::string &name) const;

    /// get a string property by its key name
    virtual void get_string(const std::string &name, std::string &out) const;

    /// build a report
    virtual void report(scopira::tool::property *p) const;

  private:

    const std::string m_msg;
};

#endif

