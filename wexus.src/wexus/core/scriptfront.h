
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

#ifndef __INCLUDED_WEXUS_CORE_SCRIPTFRONT_HPP__
#define __INCLUDED_WEXUS_CORE_SCRIPTFRONT_HPP__

#include <scopira/tool/thread.h>
#include <wexus/core/front.h>

namespace scopira
{
  namespace tool
  {
    class oflow_i;
  }
}

namespace wexus
{
  namespace core
  {
    class scriptfront;
  }
}

/**
 * a scriptable front end use to debug/test a site. 
 * initially, this will simply execute a list of urls
 * with values, but in the future this might include
 * a full scripting language (guile/scheme plugged in perhaps)
 *
 * @author Aleksander Demko
 */
class wexus::core::scriptfront : private scopira::tool::thread,
  public virtual wexus::core::front_i
{
  public:
    /// ctor
    scriptfront(void);
    /// constructor
    scriptfront(const std::string &filename, front_peer_i *peer);
    /// destructor
    virtual ~scriptfront();

    /// load params from a prop tree
    virtual bool init_prop(front_peer_i *peer, scopira::tool::property *prop);

    /// thread

    /// the run core
    virtual void run(void);

    // front_i

  protected:
    class script_front_event : public wexus::core::front_event_imp
    {
      public:
        virtual ~script_front_event() {}

        virtual void set_request(const std::string& request) { i_set_request(request); }

        virtual scopira::tool::oflow_i& get_output(void);

        virtual const std::string& get_form_field(const std::string& name) const { return name; }
        virtual bool has_form_field(const std::string& name) const { return false; }

        virtual void get_client_cookies(const std::string& name, std::vector<std::string>&) const {}
        virtual bool has_client_cookie(const std::string& name) const { return false; }
    };

  private:
    std::string m_filename; /// filename to work with, if any
};

#endif

