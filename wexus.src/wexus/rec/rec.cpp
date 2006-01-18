
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

#include <wexus/rec/rec.h>

#include <assert.h>
#include <ctype.h>

#include <scopira/tool/flow.h>
#include <scopira/tool/stringflow.h>

#include <wexus/rec/basetoken.h>

//BBtargets libwexus.so

using namespace wexus;
using namespace scopira::tool;

namespace {
void print_rec(rec::rec_i &r, oflow_i &out, int indent)
{
  int x;
  rec::rec_iterator jj;
  std::string s;

  if (r.has_string("title")) {
    for (x=0; x<indent; ++x)
      out << ' ';
    r.get_string("title", s);
    out << "title = " << s << '\n';
  }
  if (r.has_string("name")) {
    for (x=0; x<indent; ++x)
      out << ' ';
    r.get_string("name", s);
    out << "name = " << s << '\n';
  }

  // iterate over all the sub record elements
  for (jj=r.get_rec_iterator(); jj.valid(); ++jj) {
    for (x=0; x<indent; ++x)
      out << ' ';
    out << "Sub Record:\n";
    print_rec(* (*jj), out, indent+2);
  }
}
} // anonymous namespace

void rec::merge_rec_and_token(rec_i &root, token_i &tok, scopira::tool::oflow_i &out)
{
  token_i::recstack_t st;

  st.push_back(&root);

  tok.run_token(st, out);
}

void rec::merge_rec_and_string(rec_i &root, const std::string &str, scopira::tool::oflow_i &out)
{
  count_ptr< rec::token_i > tok;
  stringflow inf(str, flow_i::input_c);

  tok.set( rec::text_tokenizer()(inf) );
  assert(tok.get());

  rec::merge_rec_and_token(root, tok.ref(), out);
}

void rec::write_html_report(property *rp, oflow_i &out)
{
  const std::string *s, *subs, *subval;
  property::iterator ii;

  subval=0;//to disable that compiled warning
  out << "<table border=1>\n";
  if ( (s = rp->get_string_value("rec_name")) )
    out << "<tr><th>" << *s << "</th></tr>\n";
  if (rp->contains("string")) {
    out << "<tr><td>Strings:<ul>\n";
    for (ii = rp->get("string"); ii.valid() && (*ii)->is_node(); ++ii)
      if ( (subs = (*ii)->get_string_value("string_name")) && (subval = (*ii)->get_string_value("string_desc")) )
        out << " <li>" << *subs << " = " << *subval << '\n';
    out << "</ul></td></tr>\n";
  }
  if (rp->contains("rec")) {
    out << "<tr><td>Sub Records:<ul>\n";
    for (ii = rp->get("rec"); ii.valid() && (*ii)->is_node(); ++ii) {
      if ( (subs = (*ii)->get_string_value("string_name")) )
        out << " <li>" << *subs << " = ";
      else
        out << " <li> Unamed: = ";
      // recurse
      write_html_report((*ii), out);
    }//inner for
    out << "</ul></td></tr>\n";
  }
  out << "</table>\n";
}

scopira::tool::oflow_i& operator<<(scopira::tool::oflow_i& o, wexus::rec::rec_i &val)
{
  print_rec(val, o, 0);

  return o;
}

rec::token_i * rec::text_tokenizer::operator ()(scopira::tool::iflow_i &in)
{
  rec::list_token* lt = new rec::list_token;

  text_tokenizer_level(in, *lt);

  return lt;
}

int rec::text_tokenizer::text_tokenizer_level(scopira::tool::iflow_i& in, rec::list_token& listt, rec::ifstmt_token* ifstmt)
{
  std::string buf, prebuf;
  flow_i::byte_t by;

  while (!in.failed()) {
    // reset the buffer
    buf.clear();

    // iterate through the literals
    while ( !in.failed() && in.read_byte(by)>0 && by != '{' )
      buf.push_back(by);

    if (buf.size() > 0) {
      // we have some literal data, add it
      listt.add_token(new rec::string_token(buf));
      buf.clear();
    }

    // read the magic tag
    prebuf.clear();
    while ( !in.failed() && in.read_byte(by) ) {
      if (by == '{') { // this is for when they really just want a lone { (use {{)
        listt.add_token(new rec::string_token("{"));
        break;
      }
      else if (by == ':') { // some kind of command, save it
        prebuf = buf;
        buf.clear();
      }
      else if (by == '}') { // end command, lets decide what to do
        if (buf=="end" && prebuf.empty())
          return end_c; // exit this level
        else if (buf=="else" && prebuf.empty() && ifstmt)
        {
          // create a new else token
          rec::if_token* eltok = new rec::if_token("", true);
          // add it to the current if statement token
          ifstmt->add_if_token(eltok);
          // tokenize next level
          text_tokenizer_level(in, *eltok, ifstmt);

          return end_c; // exit this level
        }
        else if (prebuf.empty()) {
          listt.add_token(new rec::field_token(buf));
          break;
        }
        else if (prebuf == "foreach" && !buf.empty()) {
          rec::foreach_token* tok = new rec::foreach_token(buf);
          text_tokenizer_level(in, *tok);
          listt.add_token(tok);
          break;
        }
        else if ( ((prebuf=="if" || prebuf=="ifnot") ||
                  ((prebuf=="elseif" || prebuf=="elseifnot") && ifstmt))
                  && !buf.empty() )
        {
          // elseif and elseifnot can only exist within a current if statement

          if (prebuf=="if" || prebuf=="ifnot")
          {
            // create a new local if statement
            rec::ifstmt_token* local_ifstmt = new rec::ifstmt_token;

            // create a new if[not] token
            rec::if_token* iftok = new rec::if_token(buf, prebuf=="if");
            // add the if token to the current if statement
            local_ifstmt->add_if_token(iftok);
            // tokenize next level, passing the new if statement
            text_tokenizer_level(in, *iftok, local_ifstmt);

            // add if statement to list token
            listt.add_token(local_ifstmt);
          }
          else if (prebuf=="elseif" || prebuf=="elseifnot")
          {
            // we must be in the scope of an if statement
            // this is just a double check (to be safe), the
            // first check was done above.
            assert(ifstmt);

            // create a new elseif[not] token
            rec::if_token* eliftok = new rec::if_token(buf, prebuf=="elseif");
            // add it to the current if statement token
            ifstmt->add_if_token(eliftok);
            // tokenize next level
            text_tokenizer_level(in, *eliftok, ifstmt);

            return end_c; // exit this level
          }
            
          break;
        }
        else
          listt.add_token(new rec::string_token("{ERROR:" + prebuf + ":" + buf + "}" ));
      } // } close if
      else if (isalnum(by) || (by == '_') || (by == '.') || (by == '-'))
        buf.push_back(by);  // build buf
      else {
        buf.push_back(':');
        buf.push_back(by);
        buf.push_back('}');
        // literal, not a token. dump buffer and bail
        listt.add_token(new rec::string_token("{ERROR:" + buf));
        break;
      }
    } // inner magic tag while
  }//super-while

  return none_c;
}

void rec::rec_i::report(scopira::tool::property *p) const
{
  assert(p);
  p->add_end("rec_name", new property("undocumented record"));
}

rec::rec_iterator rec::rec_base::get_rec_iterator(void)
{
  return rec_iterator();
}

#ifdef HAL_DEBUG_rec
#include <scopira/tool/objflowloader.h>
#include <scopira/tool/fileflow.h>
#include <wexus/core/output.h>
#include <wexus/rec/memrec.h>
int main(int argc, char** argv)
{
#ifndef NDEBUG
  objrefcounter _duh;
#endif
  objflowloader loader;
  fileflow output(fileflow::stderr_c, 0);
  count_ptr< rec::token_i > tok;
  stringflow inf(
    "Title: \"{title}\" ({title}), by {name}\n"
    "header title: {header.title}\n"
    "footer title: {footer.title}\n"
    "\n{foreach:users} username:{uid} ({header.title}, {name})\n{end}"
    "\n\n{ifset:tr}true ({footer.title}/{footer.blah}){end}\n"
    "{ifunset:fa}false! ({title}){end}\n"
    ,flow_i::input_c);

  wexus::core::output = &output;

  rec::mem_rec rootrec, *rc, *subrc;

  rootrec.set_string("title", "home page");
  rootrec.set_string("name", "Ace");
  rootrec.set_string("tr", "1");
  rootrec.set_string("fa", "0");

  rc = new rec::mem_rec();
  rc->set_string("title", "header-t!");
  rootrec.set_rec("header", rc);
  rc = new rec::mem_rec();
  rc->set_string("title", "footer-t!");
  rootrec.set_rec("footer", rc);

  rc = new rec::mem_rec();
  rootrec.set_rec("users", rc);

  subrc = new rec::mem_rec();
  subrc->set_string("uid", "ademko");
  rc->set_rec("0", subrc);
  subrc = new rec::mem_rec();
  subrc->set_string("uid", "root");
  rc->set_rec("1", subrc);
  subrc = new rec::mem_rec();
  subrc->set_string("uid", "nobody");
  rc->set_rec("2", subrc);


  tok = rec::text_tokenizer()(inf);
  assert(tok.get());

  output << ">>>";
  rec::merge_rec_and_token(rootrec, * tok.get(), output);
  output << "<<<\n";

  output << rootrec;

  return 0; //return ok
}
#endif

