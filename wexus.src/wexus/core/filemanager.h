
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

#ifndef __INCLUDED__WEXUS_CORE_FILEMANAGER_HPP__
#define __INCLUDED__WEXUS_CORE_FILEMANAGER_HPP__

#include <string>
#include <vector>

namespace scopira
{
  namespace tool
  {
    class oflow_i;
    class time;
    class property;
  }
}

namespace wexus
{
  namespace core
  {
    class file_manager;
  }
}

/**
 * retrieves files from a set of directories.
 * a decendant class, cache_file_manager, will cache files
 * in memory
 *
 * @author Aleksander Demko
 */
class wexus::core::file_manager
{
  public:
    // load_file return codes
    enum {
      notfound_c = 0,   // not found (NAJ)
      file_c = 1,       // send file
      needslash_c = 2,  // need a trailin slash for this directory (NAJ)
      listing_c = 3,    // send directory listing
      index_c = 4,      // send index.html or something
    };

    /// default ctor
    file_manager(bool indexdirs = false, bool indexfile = false);
    /// dtor
    virtual ~file_manager();

    /// default-ctor targetted init
    virtual bool init_prop(scopira::tool::property *prop);

    /// adds a directory to the search paths
    void add_directory(const std::string &dir);

    /// returns the throttle
    int get_throttle(void) const { return m_throttle; }

    /**
     * finds the given filename, and returns a code.
     * depending on the code returned (non-NAJ), out may be the full
     * on-disk real path
     *
     * @author Aleksander Demko
     */ 
    int find_file(const std::string &filename, std::string &fullpath) const;

    /**
     * sends the given index
     *
     * @author Aleksander Demko
     */ 
    void send_index(const std::string &filename, const std::string &fullpath, scopira::tool::oflow_i &out) const;

  private:
    typedef std::vector<std::string> dirlist_t;

    dirlist_t m_dirlist; /// directory list
    int m_throttle;      /// throttle speed, 0==unlimited
    bool m_indexdir;     /// produce indicies for dirs
    bool m_indexfile;    /// check for index files like index.html
};

#endif

