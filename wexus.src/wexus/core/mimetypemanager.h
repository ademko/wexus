
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

#ifndef __INCLUDED__WEXUS_CORE_MIMETYPEMANAGER_HPP__
#define __INCLUDED__WEXUS_CORE_MIMETYPEMANAGER_HPP__

#include <string>
#include <map>

namespace wexus
{
  namespace core
  {
    class mimetype_manager;
  }
}

/**
 * Manages mime types
 *
 * This classes uses no locking, because it's mostly readonly
 *
 * @author Aleksander Demko
 */
class wexus::core::mimetype_manager
{
  public:
    /// default ctor
    mimetype_manager(void);
    /// dtor
    ~mimetype_manager();

    /// adds the bare default mime types
    void add_default_types(void);
    /// adds the types from the OS
    void add_os_types(void);
    /// adds the types from the given config file
    // placeholder
    /// adds the explicit type
    void add_type(const std::string &ext, const std::string &mime_type);

    /// gets the associated mime type given an extension  
    void get_type(const std::string& ext, std::string& mime_type) const;

  private:
    typedef std::map<std::string, std::string> mimemap_t;

    /// mime map
    mimemap_t m_mimemap;
};

#endif

