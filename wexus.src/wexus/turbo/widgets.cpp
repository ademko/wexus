
/*
 *  Copyright (c) 2006    Aleksander B. Demko
 *
 *  All rights reserved.
 *
 *  This material is confidential and proprietary information of
 *  Aleksander B. Demko ("Confidential Information").
 *  This Confidential Information may only be used and reproduced
 *  in accordance with the terms of the license agreement.
 *
 */

#include <wexus/turbo/widgets.h>


#include <scopira/tool/util.h>
#include <wexus/core/http.h>
#include <wexus/turbo/register.h>
#include <wexus/turbo/event.h>
#include <wexus/turbo/formatters.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus::db;
using namespace wexus::turbo;

//
//
// param
//
//

param::param(const std::string &name, int value)
  : m_rendered_string(name)
{
  m_rendered_string += "=";
  m_rendered_string += int_to_string(value);
}

param::param(const std::string &name, const std::string &value)
  : m_rendered_string(name)
{
  m_rendered_string += "=";
  m_rendered_string += urlenc(value);
}

param::param(const std::string &rendered_string)
  : m_rendered_string(rendered_string)
{
}

std::string param::operator +(const param &rhs) const
{
  return param(m_rendered_string + "&" + rhs.m_rendered_string);
}

//
//
// target
//
//

target::target(const std::string &fullclassname)
  : m_rendered_string(registry::instance()->get_controller_function(fullclassname, "index"))
{
}

target::target(const std::string &fullclassname, const std::string &methodname)
  : m_rendered_string(registry::instance()->get_controller_function(fullclassname, methodname.empty()?"index":methodname))
{
}

target::target(const std::string &fullclassname, const std::string &methodname, const std::string &encodedoptions)
  : m_rendered_string(registry::instance()->get_controller_function(fullclassname, methodname.empty()?"index":methodname))
{

  if (!encodedoptions.empty()) {
    m_rendered_string += "?";
    m_rendered_string += encodedoptions;
  }
}

//
//
// form
//
//

form::form(const std::string &formname, const std::string &enctargeturl,
    short type)
  : m_formname(formname)
{
  EVENT.output() << "<FORM ";

  switch (type) {
    case post_type_c: EVENT.output() << " METHOD=\"post\""; break;
    case get_type_c: EVENT.output() << " METHOD=\"get\""; break;
    //case file_type_c: EVENT.output() << " METHOD=\"post\""; break;
  }

  EVENT.output() << " ACTION=\"" << enctargeturl << "\">\n";

  //EVENT.output() << "<INPUT TYPE=\"HIDDEN\" NAME=\"formname\" VALUE=\"" << formname << "\" />\n";

  // TODO for !get_type_c, encode the encodedoptions as HIDDEN inputs, maybe
}

form::~form()
{
  EVENT.output() << "</FORM>\n";
}

std::string form::hidden_value(const std::string &fieldname, const std::string &defaultval) const
{
  std::string ret, def = my_get_form_field(fieldname);

  ret.reserve(100);

  ret += "<INPUT TYPE=\"HIDDEN\" NAME=\"";
  ret += my_field(fieldname);
  ret += "\" VALUE=\"";
  if (def.empty())
    ret += html(defaultval);
  else
    ret += html(def);
  ret += "\" />\n";

  return ret;
}

std::string form::text_field(const std::string &fieldname, const std::string &defaultval,
  int sz, int maxlen) const
{
  std::string ret, def = my_get_form_field(fieldname);

  ret.reserve(100);

  ret += "<INPUT TYPE=\"TEXT\" NAME=\"";
  ret += my_field(fieldname);
  ret += "\" VALUE=\"";
  if (def.empty())
    ret += html(defaultval);
  else
    ret += html(def);
  ret += "\" SIZE=\"";
  ret += int_to_string(sz);
  ret += "\" MAXLENGTH=\"";
  ret += int_to_string(maxlen);
  ret += "\" />\n";

  return ret;
}

std::string form::file_upload(const std::string &fieldname) const
{
  std::string ret, def = my_get_form_field(fieldname);

  ret.reserve(100);

  ret = "<INPUT TYPE=\"FILE\" NAME=\"";
  ret += my_field(fieldname);
  ret += "\" />\n";

  return ret;
}

std::string form::password_field(const std::string &fieldname, int sz, int maxlen) const
{
  std::string ret;

  ret.reserve(100);

  ret = "<INPUT TYPE=\"PASSWORD\" NAME=\"";
  ret += my_field(fieldname);
  ret += "\" SIZE=\"";
  ret += int_to_string(sz);
  ret += "\" MAXLENGTH=\"";
  ret += int_to_string(maxlen);
  ret += "\" />\n";

  return ret;
}

std::string form::text_area(const std::string &fieldname, const std::string &defaultval,
  int rows, int cols, int wraptype) const
{
  std::string ret, def = my_get_form_field(fieldname);

  ret.reserve(1024);

  ret = "<TEXTAREA NAME=\"";
  ret += my_field(fieldname);
  ret += "\" ROWS=\"";
  ret += int_to_string(rows);
  ret += "\" COLS=\"";
  ret += int_to_string(cols);
  switch (wraptype) {
    case wrap_off_c: ret += "\" WRAP=\"OFF"; break;
    case wrap_soft_c: ret += "\" WRAP=\"SOFT"; break;
    case wrap_hard_c: ret += "\" WRAP=\"HARD"; break;
  }
  ret += "\">";
  if (def.empty())
    ret += html(defaultval);
  else
    ret += html(def);
  ret += "</TEXTAREA>\n";

  return ret;
}

std::string form::checkbox(const std::string &fieldname, bool defaultchecked)
{
  std::string ret, def = my_get_form_field(fieldname);
  bool ischecked = false;

  if (def.empty())
    ischecked = defaultchecked;
  else
    ischecked = def != "0";

  ret.reserve(100);

  ret += "<INPUT TYPE=\"CHECKBOX\" NAME=\"";
  ret += my_field(fieldname);
  ret += "\" VALUE=\"1\" ";
  if (ischecked)
    ret += " CHECKED";
  ret += "/>\n";

  return ret;
}

std::string form::radio(const std::string &fieldname, const std::string &val,
  bool defaultchecked)
{
  std::string ret, def = my_get_form_field(fieldname);
  bool ischecked = false;

  if (def.empty())
    ischecked = defaultchecked;
  else
    ischecked = def == val;

  ret.reserve(100);

  ret += "<INPUT TYPE=\"RADIO\" NAME=\"";
  ret += my_field(fieldname);
  ret += "\" VALUE=\"";
  ret += html(val);
  ret += "\" ";
  if (ischecked)
    ret += " CHECKED";
  ret += "/>\n";

  return ret;
}

std::string form::submit_button(const std::string &desc, const std::string &fieldname) const
{
  std::string ret;

  ret.reserve(128);

  ret += "<INPUT TYPE=\"SUBMIT\" VALUE=\"";
  ret += html(desc);
  if (!fieldname.empty()) {
    ret += "\" NAME=\"";
    ret += my_field(fieldname);
  }
  ret += "\" />\n";

  return ret;
}

form::drop_down::drop_down(form &f, const std::string &fieldname, int viewsize)
{
  EVENT.output() << "<SELECT NAME=\"" << f.my_field(fieldname) <<
    "\" SIZE=\"" << viewsize << "\">\n";
}

form::drop_down::~drop_down()
{
  EVENT.output() << "</SELECT>\n";
}

void form::drop_down::option(const std::string &val, const std::string &desc) const
{
  EVENT.output() << "<OPTION VALUE=\"";
  EVENT.html_output() << val;
  EVENT.output() << "\">";
  EVENT.html_output() << desc;
  EVENT.output() << "</OPTION>\n";
}

std::string form::my_field(const std::string &fieldname) const
{
  if (m_formname.empty())
    return fieldname;
  else
    return m_formname + "[" + fieldname + "]";
}

const std::string & form::my_get_form_field(const std::string &fieldname) const
{
  if (m_formname.empty())
    return EVENT.get_form_field(fieldname);
  else
    return EVENT.get_form_field(m_formname + "[" + fieldname + "]");
}

//
//
// functions
//
//

void wexus::turbo::db_to_form(wexus::db::statement &s, const std::string &formname)
{
  if (!s.is_valid())
    return;

  const statement::fieldnames_t &names = s.get_fieldnames();

  for (short x=0; x<names.size(); ++x) {
    if (formname.empty())
      EVENT.set_form_field_default(names[x], s[names[x]]);
    else
      EVENT.set_form_field_default(formname + "[" + names[x] + "]", s[names[x]]);
  }
}

std::string wexus::turbo::link_to(const std::string &desc, const std::string &enctargeturl)
{
  std::string ret;

  ret.reserve(200);

  ret = "<A HREF=\"";
  ret += enctargeturl;
  ret += "\">";
  ret += html(desc);
  ret += "</A>";

  return ret;
}

std::string wexus::turbo::button_to(const std::string &desc, const std::string &enctargeturl)
{
  std::string ret;

  ret.reserve(200);

  ret = "<FORM METHOD=\"POST\" ACTION=\"";
  ret += enctargeturl;
  ret += "\"><INPUT TYPE=\"SUBMIT\" VALUE=\"";
  ret += html(desc);
  ret += "\"></FORM>";

  return ret;
}

std::string wexus::turbo::mail_to(const std::string &desc, const std::string &email, const std::string &subject)
{
  std::string ret;

  ret.reserve(100);

  ret = "<A HREF=\"mailto:";
  ret += urlenc(email);
  if (!subject.empty())
    ret += "&subject=" + urlenc(subject);
  ret += "\">";
  ret += html(desc);
  ret += "</A>";

  return ret;
}

std::string wexus::turbo::image_to(const std::string &enctargeturl)
{
  std::string ret;

  ret.reserve(100);

  ret = "<IMG SRC=\"";
  ret += enctargeturl;
  ret += "\" />";

  return ret;
}

void wexus::turbo::redirect_to(const std::string &targeturl)
{
  EVENT.front.set_redirect_url(targeturl);
}

void wexus::turbo::render_errors(void)
{
  string_iterator e = EVENT.get_errors();

  if (e.is_valid()) {
    EVENT.output() << "<h3>Form Errors!</h3><ul>\n";
    for (; e.is_valid(); ++e) {
      EVENT.output() << "<li>";
      EVENT.html_output() << *e;
      EVENT.output() << "</li>";
    }
    EVENT.output() << "</ul>\n";
  }
}

void wexus::turbo::render_notes(void)
{
  string_iterator e = EVENT.get_notes();

  if (e.is_valid()) {
    EVENT.output() << "<h3>Note!</h3><ul>\n";
    for (; e.is_valid(); ++e) {
      EVENT.output() << "<li>";
      EVENT.html_output() << *e;
      EVENT.output() << "</li>";
    }
    EVENT.output() << "</ul>\n";
  }
}

