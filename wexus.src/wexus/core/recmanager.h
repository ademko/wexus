
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

#ifndef __INCLUDED__WEXUS_CORE_RECMANAGER_HPP__
#define __INCLUDED__WEXUS_CORE_RECMANAGER_HPP__

#include <string>
#include <vector>
#include <map>

#include <scopira/tool/thread.h>

#include <wexus/rec/rec.h>
#include <wexus/rec/token.h>

namespace scopira
{
  namespace tool
  {
    class property;
    class oflow_i;
  }
}

namespace wexus
{
  namespace core
  {
    class rec_manager;
    class cache_rec_manager;
  }
}

/**
 * managers rec file merging
 *
 * @author Aleksander Demko
 */
class wexus::core::rec_manager
{
  public:
    /// default ctor
    rec_manager(void);
    /// dtor
    virtual ~rec_manager();

    /// default-ctor targetted init
    virtual bool init_prop(scopira::tool::property *prop);

    /// adds a directory to the search paths
    void add_directory(const std::string &dir);

    /**
     * template the given file, with the root rec to the given
     * output stream
     *
     * @return true if the file was found. false if not
     * @author Aleksander Demko
     */
    virtual bool merge_template(const std::string &filename,
      wexus::rec::rec_i &root, scopira::tool::oflow_i &out);

  protected:
    /**
     * find the token tree for the given filename. if non-null,
     * caller must delete. if null, then not found.
     *
     * @author Aleksander Demko
     */
    wexus::rec::token_i * find_token_tree(const std::string &filename);

  private:
    typedef std::vector<std::string> dirlist_t;

    dirlist_t m_dirlist; /// directory list
};

/**
 * rec_manager + caching
 *
 * @author Aleksander Demko
 */
class wexus::core::cache_rec_manager : public wexus::core::rec_manager
{
  public:
    /// ctor
    cache_rec_manager(void);
    /// dtor
    virtual ~cache_rec_manager(void);
    
    /// merge, as per parent
    virtual bool merge_template(const std::string &filename,
      wexus::rec::rec_i &root, scopira::tool::oflow_i &out);

    /// flush the whole cache
    void flush(void);

  private:
    typedef std::map<std::string, scopira::tool::count_ptr< wexus::rec::token_i > > cachemap_t;

    scopira::tool::rwlock m_cacherw;
    cachemap_t m_cache;       /// the cache
};

#endif

