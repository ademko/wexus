
/*
 *  Copyright (c) 2005    Aleksander B. Demko
 *
 *  All rights reserved.
 *
 *  This material is confidential and proprietary information of
 *  Aleksander B. Demko ("Confidential Information").
 *  This Confidential Information may only be used and reproduced
 *  in accordance with the terms of the license agreement.
 *
 */

#include <wexus/turbo/register.h>

#include <scopira/tool/output.h>

//BBtargets libwexus.so

using namespace wexus::turbo;

static scopira::tool::count_ptr<registry> g_the_registry;

registry * registry::instance(void)
{
  if (g_the_registry.is_null())
    g_the_registry = new registry;
  return g_the_registry.get();
}

registry::registry(void)
{
  dm_refcount = 0;
}

registry::~registry()
{
}

int registry::add_ref(void) const
{
  return ++dm_refcount;
}

bool registry::sub_ref(void) const
{
  if (--dm_refcount == 0) {
    delete this;
    return true;
  }
  assert(dm_refcount>0);
  return false;
}

void registry::add_type(const std::string & name, tfunc_t loader)
{
  m_store[name] = loader;
}

void registry::add_type(const char * name, tfunc_t loader)
{
  add_type(std::string(name), loader);
}

void registry::add_controller_type(const char *fullclassname, const char *methodname, tfunc_t loader)
{
  add_type(std::string(fullclassname) + "." + methodname + ".wx", loader);
  if (methodname == std::string("index"))
    add_type(std::string(fullclassname) + ".wx", loader);
}

std::string registry::get_controller_function(const std::string &fullclassname, const std::string &methodname)
{
  std::string ret(fullclassname + "." + methodname + ".wx");

#ifndef NDEBUG
  if (m_store.count(ret) == 0) {
    OUTPUT << "Can't find binding for fullclassname=" << fullclassname << " methodname=" << methodname << '\n';
    assert(false);
  }
#endif

  return ret;
}

registry::tfunc_t registry::get_type(const std::string &name) const
{
  funcstore_t::const_iterator ii = m_store.find(name);

  if (ii == m_store.end())
    return 0;
  else
    return ii->second;
}

