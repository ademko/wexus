
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

#include <list>

#include <scopira/tool/diriterator.h>
#include <scopira/tool/file.h>
#include <scopira/tool/fileflow.h>
#include <scopira/tool/util.h>
#include <scopira/core/loop.h>

//BBlibs wexus
//BBtargets wxmake

using namespace scopira::tool;

typedef std::vector<std::string> stringlist_t;

/**
 * TODO
 *
 * more filtering/escapes done on prints
 * use cacheflows for performance
 *
 * @author Aleksander Demko
 */ 

enum {
  html_c,
  got_open_c,
  got_open_q_c,
  got_close_q_c,
  code_c,
  code_print_c,
  include_c,
  enter_handler_c,
  preamble_on_c,
  preamble_off_c,
};

static void flush_func_start(oflow_i &out, const std::string &funcname, bool &havefunc, const stringlist_t &chunks)
{
  if (havefunc)
    return;
  havefunc = true;

  if (chunks.empty())
    out << "static void turbo_handler(wexus::turbo::turbo_event &ev)\n{\n";
  else {
    out << "void ";
    for (int x=0; x<chunks.size()-1; ++x)
      out << chunks[x] << "::";
    out << chunks[chunks.size()-1] << "(void)\n{\n";
  }
}

static bool mostly_empty(const std::string &buf)
{
  if (buf.empty())
    return true;
  if (buf.size() > 10)
    return false;

  for (int i=0; i<buf.size(); ++i)
    if (buf[i] != ' ' && buf[i] != '\t' && buf[i] != '\n' && buf[i] != '\r')
      return false;
  return true;
}

static void flush_print(oflow_i &out, const std::string &buf)
{
  out << "  EVENT.output() << \"";
  for (std::string::const_iterator ii=buf.begin(); ii != buf.end(); ++ii) {
    switch (*ii) {
      case '\n': out << "\\n"; break;
      case '\t': out << "\\t"; break;
      case '\r': out << "\\r"; break;
      case '\"': out << "\\\""; break;
      default:
        out << *ii;
    }
  }
  out << "\";\n";
}

static void flush_code(oflow_i &out, const std::string &buf)
{
  out << buf << '\n';
}

static void flush_code_print(oflow_i &out, const std::string &buf, bool html_print)
{
  if (html_print)
    out << "EVENT.html_output() << ";
  else
    out << "EVENT.output() << ";
  out << buf << ";\n";
}

static std::string clean_include_name(const std::string &filename)
{
  std::string ret(filename);

  while (!ret.empty() && ret[0] == ' ')
    ret.erase(ret.begin());

  while (!ret.empty() && ret[ret.size()-1] == ' ')
    ret.erase(ret.begin() + ret.size() - 1);

  return ret;
}

static int process_wx_imp(const std::string &wxfilename, oflow_i &outfile, bool &havefunc, bool &inpreamble, const stringlist_t &chunks)
{
  fileflow infile;
  int lineno, startlineno;
  std::string funcname;

  lineno = 1;
  infile.open(wxfilename, fileflow::input_c);

  if (infile.failed()) {
    OUTPUT << wxfilename << ": failed to open for reading\n";
    return 1;
  }

  // calc func name
  {
    std::string dummy, dummy2;

    file::split_ext(wxfilename, dummy2, dummy);
    file::split_path(dummy2, dummy, funcname);
  }

  // do the parse/conversion
  std::string buf;
  int state = html_c;
  byte_t b;
  bool state_preend = false;
  bool question_mark;
  bool html_print;

  buf.reserve(1024*32);

  while (infile.read_byte(b)) {
    if (b == '\n')
      ++lineno;
    switch (state) {
      case html_c:
        if (b == '<')
          state = got_open_c;
        else
          buf.push_back(b);
        break;
      case got_open_c:
        if (b == '?' || b == '%') {
          state = got_open_q_c;
          question_mark = b == '?';
        } else {
          buf.push_back('<');
          buf.push_back(b);
          state = html_c;
        }
        break;
      case got_open_q_c:
        if (b == 'f')
          state = enter_handler_c;
        else if (b == '=' || b == 'h') {
          state = code_print_c;
          html_print = b == 'h';
        } else if (b == 'i')
          state = include_c;
        else
          state = code_c;
        state_preend = false;
        startlineno = lineno;
        if (!mostly_empty(buf)) {
          if (!inpreamble)
            flush_func_start(outfile, funcname, havefunc, chunks);
          flush_print(outfile, buf);
        }
        buf.clear();
        break;
      case enter_handler_c:
      case code_c:
      case code_print_c:
      case include_c:
        if (state_preend) {
          if (b == '>') {
            if (state == enter_handler_c) {
              inpreamble = false;
            } else if (state == code_print_c) {
              if (!inpreamble)
                flush_func_start(outfile, funcname, havefunc, chunks);
              outfile << "#line " << startlineno << " \"" << wxfilename << "\"\n";
              flush_code_print(outfile, buf, html_print);
            } else if (state == include_c) {
              std::string p, dummy;
              int code;

              file::split_path(wxfilename, p, dummy);
              p += clean_include_name(buf);
              code = process_wx_imp(p, outfile, havefunc, inpreamble, chunks);

              if (code != 0) {
                OUTPUT << wxfilename << ":" << startlineno << ": error: included from here\n";
                return code;
              }
            } else {
              if (!inpreamble)
                flush_func_start(outfile, funcname, havefunc, chunks);
              outfile << "#line " << startlineno << " \"" << wxfilename << "\"\n";
              flush_code(outfile, buf);
            }
            buf.clear();
            state = html_c;
          } else {
            // was at preend, but didnt get the final to > after the previous ?
            buf.push_back(question_mark ? '?' : '%');
            buf.push_back(b);
            state_preend = false;
          }
        } else {
          // not preend
          if (b == '?' || b == '%') {
            state_preend = true;
            question_mark = b == '?';
          } else
            buf.push_back(b);
        }
        break;
    }//switch
  }//while

  if (!mostly_empty(buf)) {
    if (!inpreamble)
      flush_func_start(outfile, funcname, havefunc, chunks);
    flush_print(outfile, buf);
  }

  return 0;
}

static int process_wx(const std::string &wxfilename, const std::string &cppfilename)
{
  fileflow outfile;
  bool havefunc = false;
  bool inpreamble = true;
  int code;
  stringlist_t chunks;

  outfile.open(cppfilename, fileflow::output_c);

  if (outfile.failed()) {
    OUTPUT << cppfilename << ": error: failed to open for writing\n";
    return 1;
  }

  {
    std::string filename, sansext, dummy;

    file::split_path(wxfilename, dummy, filename);
    file::split_ext(filename, sansext, dummy);

    string_tokenize(sansext, chunks, ".");

    if (chunks.size() == 1)
      chunks.clear();
  }

  // header
  outfile << "\n/* wxmake compiled .wx file */\n\n#include <wexus/turbo/event.h>\n"
    "#include <wexus/turbo/usingnamespace.h>\n\n";

  if (!chunks.empty()) {
    outfile << "/* include assumed header file for this controller */\n#include <";
    for (int x=0; x<chunks.size()-2; ++x)
      outfile << chunks[x] << '/';
    outfile << chunks[chunks.size()-2] << ".h>\n\n";
  }
  
  // main body
  code = process_wx_imp(wxfilename, outfile, havefunc, inpreamble, chunks);

  // super final footer
  if (havefunc) {
    if (chunks.empty()) {
      std::string filename, dummy;

      file::split_path(wxfilename, dummy, filename);

      outfile << "}\n\nstatic wexus::turbo::register_function r1(\""
        << filename << "\", turbo_handler);\n\n";
    } else {
      outfile << "}\n\n";   // do register_controller stuff? maybe not
    }
  }

  return code;
}

static std::string replace_ext(const std::string &filename, const std::string &newext)
{
  std::string ret, dummy;

  file::split_ext(filename, ret, dummy);
  ret += newext;

  return ret;
}

static int build_make(const char *thisprog, const std::string &dir, const std::string &makefilename, const std::string &finaltarget)
{
  dir_iterator dd;
  typedef std::map<std::string, std::string> map_t;
  map_t targets;
  std::string filename;

  dd.open(dir);

  while (dd.next(filename)) {
    if (file::has_ext(filename, ".WX")) {
      targets[replace_ext(filename, ".cpp")] = filename;
      targets[replace_ext(filename, ".o")] = replace_ext(filename, ".cpp");
    }
    if (file::has_ext(filename, ".CPP"))
      targets[replace_ext(filename, ".o")] = filename;
  }

  if (targets.empty()) {
    OUTPUT << "No .cpp or .wx files found.\n";
    return 1;
  }

  fileflow outmake;

  outmake.open(makefilename, fileflow::output_c);

  if (outmake.failed()) {
    OUTPUT << makefilename << ": error: cannot open file for writing\n";
    return 1;
  }

  outmake << "\n# wxmake (v0.2) made Makefile.turbo (new style)\n\n";

  outmake <<
    "# Use BBCPP to define your own compiler\n"
    "BBCPP?=g++\n"
    "BBCC?=gcc\n"
    "\n"
    "# Use BBOPT=1 for optmized builds\n"
    "ifeq ($(strip $(BBOPT)),)\n"
    "BBCPP+= -fPIC -g\n"
    "BBCC+= -fPIC -g\n"
    "else\n"
    "BBCPP+= -fPIC -O3 -s -DNDEBUG\n"
    "BBCC+= -fPIC -O3 -s -DNDEBUG\n"
    "endif\n"
    "\n"
    "# Use BB32=1 for forced 32 bit builds\n"
    "ifneq ($(strip $(BB32)),)\n"
    "BBCPP+= -m32 -DBUILDBOSS_32\n"
    "BBCC+= -m32 -DBUILDBOSS_32\n"
    "endif\n"
    "\n"
    "# Use BB64=1 for 64 bit builds\n"
    "ifneq ($(strip $(BB64)),)\n"
    "BBCPP+= -m64 -DBUILDBOSS_64\n"
    "BBCC+= -m64 -DBUILDBOSS_64\n"
    "endif\n"
    "\n";

  outmake << "all:: " << finaltarget << "\n\n";

  outmake << finaltarget << ":";
  for (map_t::iterator ii=targets.begin(); ii != targets.end(); ++ii)
    outmake << ' ' << dir << '/' << ii->first;
  outmake << "\n\t$(BBCPP) `pkg-config --libs wexus` -o " << finaltarget;
  for (map_t::iterator ii=targets.begin(); ii != targets.end(); ++ii)
    if (file::has_ext(ii->first, ".O"))
      outmake << ' ' << dir << '/' << ii->first;
  outmake << "\n\n";

  for (map_t::iterator ii=targets.begin(); ii != targets.end(); ++ii) {
    outmake << dir << '/' << ii->first << ": " << dir << '/' << ii->second << "\n\t";
    if (file::has_ext(ii->first, ".CPP"))
      outmake << "wxmake cppfile=" << dir << '/' << ii->first << " wxfile="
      << dir << '/' << ii->second << "\n\n";
    else
      outmake << "$(BBCPP) -c `pkg-config --cflags wexus` -I" << dir << " -o "
      << dir << '/' << ii->first << ' ' << dir << '/' << ii->second << "\n\n";
  }

  outmake << "clean::\n\trm " << finaltarget;
  for (map_t::iterator ii=targets.begin(); ii != targets.end(); ++ii)
    outmake << ' ' << dir << '/' << ii->first;
  outmake << "\n\n";

  outmake << "remake::\n\t" << thisprog << " mkmakefile=" << dir
    << " makefile=" << makefilename << " target=" << finaltarget << "\n\n";

  return 0;
}

static int build_old_make(const char *thisprog, const std::string &dir, const std::string &makefilename)
{
  dir_iterator dd;
  std::string filename, cppname, dummy;
  typedef std::list<std::string> list_t;
  list_t wxfiles, cppfiles;

  dd.open(dir);

  while (dd.next(filename))
    if (file::has_ext(filename, ".WX"))
      wxfiles.push_back(filename);

  if (wxfiles.empty()) {
    OUTPUT << "No .wx files found.\n";
    return 1;
  }

  fileflow outmake;

  outmake.open(makefilename, fileflow::output_c);

  if (outmake.failed()) {
    OUTPUT << makefilename << ": error: cannot open file for writing\n";
    return 1;
  }

  outmake << "\n# wxmake (v0.1) made Makefile.turbo\n\n";

  for (list_t::iterator ii=wxfiles.begin(); ii != wxfiles.end(); ++ii) {
    file::split_ext(*ii, cppname, dummy);
    cppname += ".cpp";

    cppfiles.push_back(cppname);

    outmake << dir << '/' << cppname << ": " << dir << '/' << *ii << "\n\t" << thisprog << " wxfile="
      << dir << '/' << *ii << " cppfile=" << dir << '/' << cppname << "\n\n";
  }

  outmake << "all::";
  for (list_t::iterator ii=cppfiles.begin(); ii != cppfiles.end(); ++ii)
    outmake << ' ' << dir << '/' << *ii;

  outmake << "\n\n";

  outmake << "clean::\n\trm ";
  for (list_t::iterator ii=cppfiles.begin(); ii != cppfiles.end(); ++ii)
    outmake << dir << '/' << *ii << ' ';
  outmake << "\n\n";

  outmake << "cleanwx:\n\trm ";
  for (list_t::iterator ii=cppfiles.begin(); ii != cppfiles.end(); ++ii)
    outmake << dir << '/' << *ii << ' ';
  outmake << "\n\n";

  outmake << "rewx::\n\t" << thisprog << " dir=" << dir << "\n\n";

  return 0;
}

std::string get_realpath(const std::string &path)
{
  char buf[PATH_MAX+2];   // just to be sure
  char *ret;

  ret = ::realpath(path.c_str(), buf);
  assert(ret);

  return ret;
}

int make_app(const std::string &appname)
{
  //TODO support namespace::encodings in the appname thing?
  //TODO optional force-overwrite mode?

  if (file(appname + ".h").exists())
    OUTPUT << appname << ".h already exists\n";
  else {
    fileflow outf(appname + ".h", fileflow::output_c);

    if (outf.failed()) {
      OUTPUT << "Error opening " << appname << ".h\n";
      return 1;
    }

    outf << 
      "\n\n#ifndef __INCLUDED_" << appname << "_APP_H__\n"
      "#define __INCLUDED_" << appname << "_APP_H__\n\n"

      "#include <wexus/core/app.h>\n"
      "#include <wexus/core/sessionmanager.h>\n"
      "#include <wexus/db/pool.h>\n\n"

      "class " << appname << "_app : public wexus::core::app\n"
      "{\n"
      "  public:\n"
      "    " << appname << "_app(void);\n\n"

      "  private:\n"
      "    wexus::core::session_manager m_sesmgr;\n"
      "    wexus::db::connection_pool m_dbpoolmgr;\n"
      "};\n\n"
      "#endif\n\n";

    OUTPUT << appname << ".h created\n";
  }

  if (file(appname + ".cpp").exists())
    OUTPUT << appname << ".cpp already exists\n";
  else {
    fileflow outf(appname + ".cpp", fileflow::output_c);

    if (outf.failed()) {
      OUTPUT << "Error opening " << appname << ".cpp\n";
      return 1;
    }

    outf << 
      "\n#include <" << appname << ".h>\n\n"

      "#include <wexus/core/loop.h>\n"
      "#include <wexus/core/engine.h>\n"
      "#include <wexus/core/httpfront.h>\n"
      "#include <wexus/core/defaultreactor.h>\n"
      "#include <wexus/core/quitfront.h>\n"
      "#include <wexus/turbo/turboreactor.h>\n\n"

      "" << appname << "_app::" << appname << "_app(void)\n"
      "  : m_sesmgr(\"" << appname << "SESID\", \"/" << appname << "\", \"\")\n"
      "{\n"
      "  bool b;\n\n"

      "  m_sesmgr.set_file_store(\"/tmp\");\n\n"

      "  b = m_dbpoolmgr.init_pool(2, \"ademko\", \"ademko\", \"\");\n"
      "  assert(b && \"[db failed to open]\");\n\n"

      "  wexus::turbo::turbo_reactor *tr;\n\n"

      "  add_reactor(tr = new wexus::turbo::turbo_reactor(&m_sesmgr, &m_dbpoolmgr));\n"
      "  add_reactor(new wexus::core::default_reactor(\"notfound.wx\", tr));\n"
      "}\n\n"

      "int main(int argc, char **argv)\n"
      "{\n"
      "  wexus::core::wexus_loop sh(argc, argv);\n"
      "  wexus::core::engine eng;\n\n"

      "  eng.add_front(new wexus::core::http_front(&eng, 5, 8080));\n"
      "  //eng.add_front(new wexus::core::file_quitfront(&eng, \"/tmp/quit\"));\n"
      "  eng.add_app(\"/" << appname << "\", new " << appname << "_app);\n\n"

      "  eng.start();\n"
      "  eng.wait_stop();\n\n"

      "  return 0;\n"
      "}\n\n";

    OUTPUT << appname << ".cpp created\n";
  }
  return 0;
}

int make_controller(const std::string &controllername, const std::string &methods)
{
  if (file(controllername + "_co.h").exists())
    OUTPUT << controllername << "_co.h already exists\n";
  else {
    fileflow outf(controllername + "_co.h", fileflow::output_c);

    if (outf.failed()) {
      OUTPUT << "Error opening " << controllername << "_co.h\n";
      return 1;
    }

    outf << 
      "#ifndef __INCLUDED_" << controllername << "_CO_CONTROLLER_H__\n"
      "#define __INCLUDED_" << controllername << "_CO_CONTROLLER_H__\n\n"

      "#include <wexus/turbo/event.h>\n"
      "#include <wexus/turbo/controller.h>\n\n"

      "class " << controllername << "_co : public wexus::turbo::turbo_controller\n"
      "{\n"
      "  private:\n"
      "    typedef turbo_controller parent_type;\n"
      "    typedef " << controllername << "_co this_type;\n\n"

      "  public:\n"
      "    " << controllername << "_co(void);\n\n"

      "    void index(void);\n\n"

      "};\n\n"
      "#endif\n\n";

    OUTPUT << controllername << "_co.h created\n";
  }

  if (file(controllername + "_co.cpp").exists())
    OUTPUT << controllername << "_co.cpp already exists\n";
  else {
    fileflow outf(controllername + "_co.cpp", fileflow::output_c);

    if (outf.failed()) {
      OUTPUT << "Error opening " << controllername << "_co.cpp\n";
      return 1;
    }

    outf << 
      "\n"
      "#include <" << controllername << "_co.h>\n\n"

      "using namespace scopira::tool;\n"
      "using namespace wexus::db;\n"
      "using namespace wexus::turbo;\n\n"

      "static wexus::turbo::register_controller<" << controllername
      << "_co, &" << controllername << "_co::index> r1(\"" << controllername << "_co\", \"index\");\n\n"

      << controllername << "_co::" << controllername << "_co(void)\n"
      "{\n"
      "}\n\n"
      "void " << controllername << "_co::index(void)\n"
      "{\n"
      "  EVENT.output() << \"Hello, world\";\n"
      "};\n\n";

    OUTPUT << controllername << "_co.cpp created\n";
  }

  std::vector<std::string> methlist;
  string_tokenize(methods, methlist, ",");

  for (int m=0; m<methlist.size(); ++m) {
    std::string filename = controllername + "_co." + methlist[m] + ".wx";
    if (file(filename).exists()) {
      OUTPUT << filename << " already exists\n";
      continue;
    }

    fileflow outf(filename, fileflow::output_c);

    if (outf.failed()) {
      OUTPUT << "Error opening " << filename << '\n';
      return 1;
    }

    outf << "\n<?f?>\n\n<? standard_header H; ?>\n<? render_errors(); ?>\n<? render_notes(); ?>\n\n<h1>Title</h1>\n\nHello, world!\n\n";

    OUTPUT << filename << " created\n";
  }//for

  return 0;
}

int main(int argc, char **argv)
{
  scopira::core::basic_loop sh(argc, argv);

  sh.set_config_default("makefile", "Makefile.turbo");
  sh.set_config_default("target", "wxapp");

  if (sh.has_config("wxfile") && sh.has_config("cppfile"))
    return process_wx(sh.get_config("wxfile"), sh.get_config("cppfile"));
  else if (sh.has_config("mkmakefile"))
    return build_make(argv[0], get_realpath(sh.get_config("mkmakefile")), sh.get_config("makefile"), sh.get_config("target"));
  else if (sh.has_config("dir"))
    return build_old_make(argv[0], get_realpath(sh.get_config("dir")), sh.get_config("makefile"));
  else if (sh.has_config("mkapp"))
    return make_app(sh.get_config("mkapp"));
  else if (sh.has_config("mkcontroller"))
    return make_controller(sh.get_config("mkcontroller"), sh.get_config("mkmethod"));
  else {
    OUTPUT << "wxmake - Wexus Make; a Turbo inspired code generator, compiler and helper\n\n"
      "Scaffolding source code generation:\n"
      "  mkmakefile=dirwithsourcefiles  Build a makefile for the given dir of .cpp and/or .wx files\n"
      //"  dir=dirfiledwithwxfiles     (Optional and deprecated) Use old style makefile generation\n"
      "    target=webapp                  (Optional) The name of the executable that will be built\n"
      "    makefile=Makefile.turbo        (Optional) The makefile that will be made by the dir= version\n\n"
      "  mkapp=appname                  Generate the core application-class code\n"
      "\n"
      "  mkcontroller=controllername    Generate a controller class\n"
      "    mkmethod=methods,methods       (Optional) Generate some wx-able methods for that controller\n"
      "\n"
      "To compile on .wx file into a .cpp file:\n"
      "  wxfile=inputfilename\n"
      "  cppfile=outputfilename\n\n"
      "Turbo-script mark-up language reference:\n"
      "  <?f?>           After this, we're in the body function\n"
      "  <?i filename ?> Include a given file \n"
      "  <?= expr ?>     Print the expr via <<\n"
      "  <?h expr ?>     Print the expr via << (HTML encoded)\n"
      "  <? code ?>      Code snippets\n"
      "  <% %>           These can be used instead of <? ?>\n\n";
    return 0;
  }
}

