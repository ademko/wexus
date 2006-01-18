
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

#include <wexus/rec/html.h>

#include <wexus/core/http.h>
#include <scopira/tool/util.h>

//BBtargets libwexus.so

using namespace wexus;

rec::form_hrec::form_hrec(const std::string& name)
  : m_name(name), m_method(none_c)
{
}

rec::form_hrec::form_hrec(const std::string& name, const std::string &action, int method)
  : m_name(name), m_actionurl(action)
{
  // use the action-less ctor instead if using none_c
  assert(method != none_c);

  if (method == fileupload_c) {
    m_method = post_c;
    m_upload = true;
  } else {
    m_method = method;
    m_upload = false;
  }
}

/// does the rec have this particular string prop?
bool rec::form_hrec::has_string(const std::string &name) const
{
  return (name == "begin") || (name == "end") || (name == "string")
    || (name == "html") || mem_rec::has_string(name);
}

/// get a string property by its key name
void rec::form_hrec::get_string(const std::string &name, std::string &out) const
{
  if (name == "begin") {

    // create start of tag
    out = "<form";

    // set form name
    if (!m_name.empty())
      out += (" name=\""+m_name+"\"");

    // set form action and method
    if (!m_actionurl.empty()) {
      std::string tmp;
      core::html_quote_encode(m_actionurl, tmp);

      out += std::string(" action=\"") + tmp
        + (m_upload?"\" enctype=\"multipart/form-data":"")
        + "\" method=\"" + (m_method == post_c ? "post" : "get") + "\"";
    }

    // append end of tag and any hidden fields
    out += (">" + m_hiddens);
    return;
  }

  if (name == "end" ) {
    out = "</form>";
    return;
  }

  if ( (name == "string") ||  (name == "html") ) {
    std::string tmp;
    //rec_iterator ii;

    // build the full form
    get_string("begin", out);
    out += "Complete form representation here.\n";
    /*for (ii = get_rec_iterator(); ii.valid(); ++ii) {
      (*ii)->get_string("html", tmp);
      out += tmp;
    }*/
    get_string("end", tmp);
    out += tmp;

    return;
  }

  // nothing? pass on
  mem_rec::get_string(name, out);
}

void rec::form_hrec::report(scopira::tool::property *p) const
{
  p->add_end("rec_name", new scopira::tool::property("edit form"));
  mem_rec::report(p);
}

void rec::form_hrec::add_hidden(const std::string &name, const std::string &value)
{
  std::string tmp, tmp2;

  core::html_quote_encode(name, tmp);
  core::html_quote_encode(value, tmp2);
  m_hiddens += "<input type=\"hidden\" name=\"" + tmp + "\" value=\"" + tmp2 + "\" />\n";
}

rec::button_hrec::button_hrec(int type, const std::string &desc)
  : m_type(type), m_desc(desc)
{
}

bool rec::button_hrec::has_string(const std::string &name) const
{
  return (name == "begin") || (name == "end") || (name == "string") || (name == "html");
}

void rec::button_hrec::get_string(const std::string &name, std::string &out) const
{
  if (name == "begin") {
    out = std::string("<input type=\"") + (m_type == reset_c ? "reset" : "submit") + "\" ";
    if (!m_desc.empty()) {
      std::string tmp;

      out += "value=\"";
      core::html_quote_encode(m_desc, tmp);       // should we be caching this conversion?
      out += tmp + "\" ";
    }

    return;
  }
  if (name == "end" ) {
    out = " />";  // / required for XHTML compliance
    return;
  }
  if ( (name == "string") ||  (name == "html") ) {
    std::string tmp;

    get_string("begin", out);
    get_string("end", tmp);
    out += tmp;

    return;
  }

  // nothing? then we shouldn't have been called!
  assert(false);
}

void rec::button_hrec::report(scopira::tool::property *p) const
{
  p->add_end("rec_name", new scopira::tool::property("button widget"));
}

rec::checkbox_hrec::checkbox_hrec(const std::string &name, const std::string &val,
      bool checked, const std::string &desc)
  : m_name(name), m_val(val), m_desc(desc), m_checked(checked)
{
}

bool rec::checkbox_hrec::has_string(const std::string &name) const
{
  return (name == "begin") || (name == "end") || (name == "string") || (name =="html");
}

void rec::checkbox_hrec::get_string(const std::string &name, std::string &out) const
{
  if (name == "begin") {
    std::string tmp, tmp2;

    core::html_quote_encode(m_name, tmp);
    core::html_quote_encode(m_val, tmp2);
    out = "<input type=\"checkbox\" name=\""
      + tmp + "\" value=\"" + tmp2 + "\"";
    
    if (m_checked)
      out += "checked ";
    else
      out += ' ';
    return;
  }
  if (name == "end" ) {
    out = " />";  // / required for XHTML compliance
    return;
  }
  if ( (name == "string") ||  (name == "html") ) {
    std::string tmp;

    get_string("begin", out);
    get_string("end", tmp);
    out += tmp;

    return;
  }
  if (name == "desc") {
    out = m_desc;
    return;
  }

  // nothing? then we shouldn't have been called!
  assert(false);
}

void rec::checkbox_hrec::report(scopira::tool::property *p) const
{
  p->add_end("rec_name", new scopira::tool::property("checkbox widget"));
}

rec::fileupload_hrec::fileupload_hrec(const std::string &name)
  : m_name(name)
{
}

bool rec::fileupload_hrec::has_string(const std::string &name) const
{
  return (name == "begin") || (name == "end") || (name == "string") || (name == "html");
}

void rec::fileupload_hrec::get_string(const std::string &name, std::string &out) const
{
  if (name == "begin") {
    std::string tmp;

    core::html_quote_encode(m_name, tmp);
    out = std::string("<input type=\"file\" name=\"") + tmp + "\" ";
    
    return;
  }
  if (name == "end" ) {
    out = " />";  // / required for XHTML compliance
    return;
  }
  if ( (name == "string") ||  (name == "html") ) {
    std::string tmp;

    get_string("begin", out);
    get_string("end", tmp);
    out += tmp;

    return;
  }

  // nothing? then we shouldn't have been called!
  assert(false);
}

void rec::fileupload_hrec::report(scopira::tool::property *p) const
{
  p->add_end("rec_name", new scopira::tool::property("file-upload widget"));
}

rec::hidden_hrec::hidden_hrec(const std::string &name, const std::string &val)
  : m_name(name), m_val(val)
{
}

bool rec::hidden_hrec::has_string(const std::string &name) const
{
  return (name == "begin") || (name == "end") || (name == "string") || (name == "html");
}

void rec::hidden_hrec::get_string(const std::string &name, std::string &out) const
{
  if (name == "begin") {
    std::string tmp, tmp2;

    core::html_quote_encode(m_name, tmp);
    core::html_quote_encode(m_val, tmp2);
    out = "<input type=\"hidden\" name=\""
      + tmp + "\" value=\"" + tmp2 + "\" ";
    
    return;
  }
  if (name == "end" ) {
    out = " />";  // / required for XHTML compliance
    return;
  }
  if ( (name == "string") ||  (name == "html") ) {
    std::string tmp;

    get_string("begin", out);
    get_string("end", tmp);
    out += tmp;

    return;
  }

  // nothing? then we shouldn't have been called!
  assert(false);
}

void rec::hidden_hrec::report(scopira::tool::property *p) const
{
  p->add_end("rec_name", new scopira::tool::property("hidden widget"));
}

rec::image_hrec::image_hrec(const std::string &url)
  : m_url(url)
{
}

bool rec::image_hrec::has_string(const std::string &name) const
{
  return (name == "begin") || (name == "end") || (name == "string") || (name == "html");
}

void rec::image_hrec::get_string(const std::string &name, std::string &out) const
{
  if (name == "begin") {
    std::string tmp;

    core::html_quote_encode(m_url, tmp);
    out = std::string("<img src=\"") + tmp + "\" ";
    
    return;
  }
  if (name == "end" ) {
    out = " />"; // / required for XHTML compliance
    return;
  }
  if ( (name == "string") ||  (name == "html") ) {
    std::string tmp;

    get_string("begin", out);
    get_string("end", tmp);
    out += tmp;

    return;
  }

  // nothing? then we shouldn't have been called!
  assert(false);
}

void rec::image_hrec::report(scopira::tool::property *p) const
{
  p->add_end("rec_name", new scopira::tool::property("image widget"));
}

rec::link_hrec::link_hrec(const std::string &url, const std::string &desc)
  : m_url(url), m_desc(desc)
{
}

bool rec::link_hrec::has_string(const std::string &name) const
{
  return (name == "begin") || (name == "end") || (name == "string") || (name == "html");
}

void rec::link_hrec::get_string(const std::string &name, std::string &out) const
{
  std::string tmp;
  if (name == "begin") {
    core::html_quote_encode(m_url, tmp);
    out = std::string("<a href=\"") + tmp + "\" ";
    return;
  }
  if (name == "end" ) {
    core::html_quote_encode(m_desc, tmp);
    out = std::string(">") + tmp + "</a>";
    return;
  }
  if ( (name == "string") ||  (name == "html") ) {
    get_string("begin", out);
    get_string("end", tmp);
    out += tmp;

    return;
  }

  // nothing? then we shouldn't have been called!
  assert(false);
}

void rec::link_hrec::report(scopira::tool::property *p) const
{
  p->add_end("rec_name", new scopira::tool::property("url-link widget"));
}

rec::list_hrec::list_hrec(const std::string &name, int sz, bool multi)
  : m_name(name), m_size(sz), m_multi(multi)
{
}

bool rec::list_hrec::has_string(const std::string &name) const
{
  return (name == "begin") || (name == "end") || (name == "string") || (name =="html");
}

void rec::list_hrec::get_string(const std::string &name, std::string &out) const
{
  std::string tmp;

  if (name == "begin") {
    core::html_quote_encode(m_name, tmp);

    out = std::string("<select name=\"") + tmp + "\" size=\"" +
      scopira::tool::int_to_string(m_size) + "\" ";
    if (m_multi)
      out += "multiple ";

    return;
  }
  if (name == "end") {
    optlist_t::const_iterator ii;
    out = ">\n";

    for (ii=m_options.begin(); ii != m_options.end(); ++ii) {
      core::html_quote_encode( (*ii).value, tmp);
      out += "<option value=\"" + tmp + "\" ";
      if ((*ii).selected)
        out += "selected";
      core::html_quote_encode( (*ii).desc, tmp);
      out += std::string(">") + tmp + "</option>\n";
    }

    out += "</select>\n";
    return;
  }
  if ( (name == "string") ||  (name == "html") ) {

    get_string("begin", out);
    get_string("end", tmp);
    out += tmp;

    return;
  }

  // nothing? then we shouldn't have been called!
  assert(false);
}

void rec::list_hrec::report(scopira::tool::property *p) const
{
  p->add_end("rec_name", new scopira::tool::property("list widget"));
}

void rec::list_hrec::add_option(const std::string &val, const std::string &desc, bool selected)
{
  m_options.push_back(option_t(val, desc, selected));
}

void rec::list_hrec::add_option(rec_iterator &ii, const std::string &varfield, const std::string descfield)
{
  option_t opt;

  while (ii.valid()) {
    assert( (*ii)->has_string(varfield) );
    assert( (*ii)->has_string(descfield) );

    (*ii)->get_string(varfield, opt.value);
    (*ii)->get_string(descfield, opt.desc);

    m_options.push_back(opt);
  }
}

void rec::list_hrec::set_selected(const std::string &val)
{
  optlist_t::iterator ii;

  for (ii=m_options.begin(); ii != m_options.end(); ++ii)
    if ( (*ii).value == val )
      (*ii).selected = true;
}

rec::radio_hrec::radio_hrec(const std::string &name, const std::string &val,
      bool checked, const std::string &desc)
  : m_name(name), m_val(val), m_desc(desc), m_checked(checked)
{
}

bool rec::radio_hrec::has_string(const std::string &name) const
{
  return (name == "begin") || (name == "end") || (name == "string") || (name == "html") || (name == "desc");
}

void rec::radio_hrec::get_string(const std::string &name, std::string &out) const
{
  if (name == "begin") {
    std::string tmp, tmp2;

    core::html_quote_encode(m_name, tmp);
    core::html_quote_encode(m_val, tmp2);
    out = "<input type=\"radio\" name=\""
      + tmp + "\" value=\"" + tmp2 + "\"";
    
    if (m_checked)
      out += "checked ";
    else
      out += ' ';
    return;
  }
  if (name == "end" ) {
    //out = " />";  // / required for XHTML compliance
    out = ">";
    out += m_desc;
    out += "</input>";
    return;
  }
  if ( (name == "string") ||  (name == "html") ) {
    std::string tmp;

    get_string("begin", out);
    get_string("end", tmp);
    out += tmp;

    return;
  }
  if (name == "desc") {
    out = m_desc;
    return;
  }

  // nothing? then we shouldn't have been called!
  assert(false);
}

void rec::radio_hrec::report(scopira::tool::property *p) const
{
  p->add_end("rec_name", new scopira::tool::property("radio button widget"));
}

rec::textfield_hrec::textfield_hrec(const std::string &name, const std::string &def,
      int sz, int maxlen, int type)
  : m_name(name), m_def(def), m_size(sz), m_maxlen(maxlen), m_type(type)
{
}

bool rec::textfield_hrec::has_string(const std::string &name) const
{
  return (name == "begin") || (name == "end") || (name == "string") || (name =="html");
}

void rec::textfield_hrec::get_string(const std::string &name, std::string &out) const
{
  if (name == "begin") {
    std::string tmp;

    out = "<input type=\"";
    if (m_type == password_c)
      out += "password";
    else
      out += "text";

    out += "\" name=\"";
    core::html_quote_encode(m_name, tmp);
    out += tmp;
    out += "\" value=\"";
    core::html_quote_encode(m_def, tmp);
    out += tmp + "\" size=\"" + scopira::tool::int_to_string(m_size) + "\" maxlength=\""
      + scopira::tool::int_to_string(m_maxlen) + "\" ";
    
    return;
  }
  if (name == "end" ) {
    out = " />";  // / required for XHTML compliance
    return;
  }
  if ( (name == "string") ||  (name == "html") ) {
    std::string tmp;

    get_string("begin", out);
    get_string("end", tmp);
    out += tmp;

    return;
  }

  // nothing? then we shouldn't have been called!
  assert(false);
}

void rec::textfield_hrec::report(scopira::tool::property *p) const
{
  p->add_end("rec_name", new scopira::tool::property("textfield widget"));
}

rec::textarea_hrec::textarea_hrec(const std::string &name, const std::string &def,
      int rows, int cols, int wrap)
  : m_name(name), m_def(def), m_rows(rows), m_cols(cols), m_wrap(wrap)
{
}

bool rec::textarea_hrec::has_string(const std::string &name) const
{
  return (name == "begin") || (name == "end") || (name == "string") || (name =="html");
}

void rec::textarea_hrec::get_string(const std::string &name, std::string &out) const
{
  std::string tmp;
  if (name == "begin") {
    core::html_quote_encode(m_name, tmp);
    out = std::string("<textarea name=\"") + tmp
        + "\" rows=\"" + scopira::tool::int_to_string(m_rows) + "\" cols=\""
        + scopira::tool::int_to_string(m_cols) + "\"";
    
    if (m_wrap != none_c) {
      out += " wrap=\"";

      switch (m_wrap) {
        case off_c: out += "off\" "; break;
        case soft_c: out += "soft\" "; break;
        case hard_c: out += "hard\" "; break;
      }
    }

    return;
  }
  if (name == "end" ) {
    out = '>';
    core::html_tag_encode(m_def, tmp);
    out += tmp + "</textarea>";
    return;
  }
  if ( (name == "string") ||  (name == "html") ) {
    get_string("begin", out);
    get_string("end", tmp);
    out += tmp;

    return;
  }

  // nothing? then we shouldn't have been called!
  assert(false);
}

void rec::textarea_hrec::report(scopira::tool::property *p) const
{
  p->add_end("rec_name", new scopira::tool::property("textarea widget"));
}

