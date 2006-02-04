
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

#include <wexus/core/engine.h>

#include <assert.h>

#include <scopira/tool/fileflow.h>
#include <scopira/tool/netflow.h>
#include <scopira/tool/propflow.h>
#include <scopira/tool/prop.h>
#include <scopira/tool/util.h>
#include <scopira/tool/objflowloader.h>

#include <wexus/core/output.h>
#include <wexus/core/string.h>

//BBlibs scopira
//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus;

// singleton thing - a tad annoying
core::engine* core::engine::dm_instance;

const std::string * core::empty_string;

core::engine::engine(void)
{
  assert(dm_instance == 0);
  dm_instance = this;

  m_shutdown.pm_data = false;

  assert(!empty_string);
  empty_string = new std::string;

  m_mimeman.add_default_types();
  m_mimeman.add_os_types();
}

core::engine::~engine()
{
  assert(empty_string);
  delete empty_string;
  empty_string = 0;

  assert(dm_instance == this);
  dm_instance = 0;
}

void core::engine::handle_front_event(front_event_i& evt)
{
  applist_t::iterator jj;

  // for now, just iterate through the keys looking for a match.
  // this is lazy, inefficient and non-scalable and will be replaced
  // by a more proper and flexible system soon enough.
  // hmm, it depends. if we tipically have only a few apps,
  // then a list-iteration would be better than a map look up.
  // if we have many, then well need a map

  OUTPUT << " engine \"" << evt.get_request() << '\"';

  // check for empty
  if (evt.get_request().empty()) {
    // do default handling
    evt.get_output() << "Empty request?\n";
    return;
  }

  // check if we dont hany / in the string (ignoring the first one)
  // if we dont, redirect em to one
  if (evt.get_request().size() > 1 && evt.get_request().find('/', 1) == std::string::npos) {
    evt.set_redirect_url(evt.get_request() + "/");
    return;
  }

  // "/" is a valid request

#ifndef NDEBUG
  if (evt.get_request() == "/quit/") {
    // remove this in real code FIXME (to be replaced by a reactor)
    // or filter to only allow from localhost?
    OUTPUT << "Quit triggered.\n";
    handle_shutdown();
    return;
  }
#endif

  for (jj=m_apps.begin(); jj != m_apps.end(); ++jj)
#if defined(__GNUC__) && (__GNUC__ < 3)
    // gnu c++ only has this, under rh71 anyways, ugh
    if (evt.get_request().compare((*jj).first, 0, (*jj).first.length()) == 0) {
#else
    if (evt.get_request().compare(0, (*jj).first.length(), (*jj).first) == 0) {
#endif
      std::string subreq(evt.get_request().substr((*jj).first.length()));
      app_event_data appd(evt.get_request());
      app_event appy(subreq, evt, appd);

      (*jj).second->handle_app_event(appy); // process the event with the mounted app
      
      return; // were done
    }
//}

  //fall back
  evt.get_output() << "No application found for this request: " << evt.get_request() << "\n";
}

void core::engine::handle_shutdown(void)
{
  notify_stop();

  scopira::tool::locker_ptr<bool> lk_ptr(m_shutdown);

  *lk_ptr = true;
  m_shutdown.pm_condition.notify_all();
}

bool core::engine::load_prop_file(const std::string &filename, scopira::tool::property *out)
{
  fileflow foo(filename, fileflow::input_c);
  commentiflow cfoo(false, &foo);

  if (foo.failed())
    return false;

  return load_prop_file(cfoo, out);
}


bool core::engine::load_prop_file(iflow_i &in, property *out)
{
  propiflow inp(false, &in);
  property::iterator jj;
  const std::string *s;
  int x;

  assert(out);

  if (!inp.read_property(out))
    return false;

  //OUTPUT << "READ_PROPERTY_TREE:\n" << out << '\n';
  s = out->get_string_value("wexus_file_version");
  if (!s || !string_to_int(*s, x) || x>1) {
    OUTPUT << "wexus_file_version of 1 or less required\n";
    return false;
  }

  // load all the libraries
  for (jj = out->get("lib"); jj.valid(); ++jj)
    load_lib((*jj)->get_value_as_string());

  // load all the fronts
  for (jj = out->get("front"); jj.valid(); ++jj)
    if ((*jj)->is_node())
      load_front(*jj);
  // load all the apps
  for (jj = out->get("app"); jj.valid(); ++jj)
    if ((*jj)->is_node())
      load_app(*jj);

  return true;
}

void core::engine::add_front(front_i* front)
{
  m_fronts.push_back(front);
}

void core::engine::add_app(const std::string &mntpoint, app *a)
{
  m_apps.push_back(appentry_t(mntpoint + "/", a));
  //m_apps[mntpoint] = a;
  OUTPUT << "engine started app=" << mntpoint << '\n';
}

void core::engine::start(void)
{
  locker k(m_shutdown.pm_mutex);
  frontlist_t::iterator ii;
  applist_t::iterator jj;

  // starts all the fronts
  for (ii=m_fronts.begin(); ii != m_fronts.end(); ++ii)
    (*ii)->start();
  // start all the apps
  for (jj=m_apps.begin(); jj != m_apps.end(); ++jj)
    (*jj).second->start();
}

void core::engine::notify_stop(void)
{
  locker k(m_shutdown.pm_mutex);
  frontlist_t::iterator ii;
  applist_t::iterator jj;

  for (ii=m_fronts.begin(); ii != m_fronts.end(); ii++)
    (*ii)->notify_stop();
  for (jj=m_apps.begin(); jj != m_apps.end(); ++jj)
    (*jj).second->notify_stop();
}

void core::engine::wait_stop(void)
{
  locker_ptr<bool> lk_ptr(m_shutdown);
  frontlist_t::iterator ii;
  applist_t::iterator jj;

  if (*lk_ptr == true)
    return;

  // block until were notified of death
  m_shutdown.pm_condition.wait(m_shutdown.pm_mutex);

  for (ii=m_fronts.begin(); ii != m_fronts.end(); ii++)
    (*ii)->wait_stop();
  for (jj=m_apps.begin(); jj != m_apps.end(); ++jj)
    (*jj).second->wait_stop();
}

bool core::engine::is_running(void) const
{
  locker k(m_shutdown.pm_mutex);
  frontlist_t::const_iterator ii;
  applist_t::const_iterator jj;

  for (ii=m_fronts.begin(); ii != m_fronts.end(); ii++)
    if ((*ii)->is_running())
      return true;
  for (jj=m_apps.begin(); jj != m_apps.end(); ++jj)
    if ((*jj).second->is_running())
      return true;

  return false;
}

object * core::engine::load_object(const std::string &name)
{
  objflowloader * ld = objflowloader::instance();

  // quick check
  if (ld->has_typeinfo(name))
    return ld->load_object(ld->get_typeinfo(name));


  const char *prepaths[] = {"wexus::core::", "wexus::", 0};
  const char **curpre;

  for (curpre = prepaths; *curpre; ++curpre) {
    std::string working(*curpre);
    working += name;

    if (ld->has_typeinfo(working))
      return ld->load_object(ld->get_typeinfo(working));
  }

  return 0; // not found
}

void core::engine::load_lib(const std::string &s)
{
  dll *d;

  OUTPUT << "Loading DLL: " << s << '\n';
  m_dlls.push_back( d = new dll);
  d->load_dll(s);
}

void core::engine::load_front(property *rp)
{
  const std::string *s;
  count_ptr< object > o;
  core::front_i *fr;

  s = rp->get_string_value("type");
  if (!s)
    return;

  OUTPUT << "Loading front: " << *s;

  o.set( load_object(*s) );
  if (!o.get()) {
    OUTPUT << " failed to find type\n";
    return; // null loaded object... whatever
  }
  fr = dynamic_cast<core::front_i*>(o.get());
  if (!fr) {
    OUTPUT << " type is NOT a front\n";
    return;
  }
  if (!fr->init_prop(this, rp)) {
    OUTPUT << " failed property loading\n";
    return;
  } else
    add_front(fr);
  OUTPUT << '\n';
}

void core::engine::load_app(property *rp)
{
  const std::string *s, *fmnt;
  count_ptr< object > o;
  core::app *a;
  property::iterator jj;

  s = rp->get_string_value("type");
  fmnt = rp->get_string_value("baseurl");
  if (!s || !fmnt)
    return;

  OUTPUT << "Loading app: " << *s;

  o.set( load_object(*s) );
  if (!o.get()) {
    OUTPUT << " failed to find type\n";
    return; // null loaded object... whatever
  }
  a = dynamic_cast<core::app*>(o.get());
  if (!a) {
    OUTPUT << " type is NOT an app\n";
    return;
  }
  if (!a->init_prop(*fmnt, rp)) {
    OUTPUT << " failed property loading\n";
    return;
  } else
    for (jj=rp->get("baseurl"); jj.valid(); ++jj) {
      OUTPUT << " @" << (*jj)->get_value_as_string();
      add_app((*jj)->get_value_as_string(), a);
    }

  OUTPUT << '\n';
}

