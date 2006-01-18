
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

#include <wexus/turbo/formatters.h>

#include <scopira/tool/util.h>

//BBtargets libwexus.so

using namespace scopira::tool;
using namespace wexus::turbo;

namespace { class markup
{
  public:
    markup(const std::string &_input, std::string &_output, unsigned int _flags);

    void go(void);

  private:
    void do_para(void);
    void do_bold(void);
    void do_italic(void);
    void do_link(void);
    void do_list(void);
    void do_numlist(void);
    void do_title(void);
    void do_codequote(void);
    void do_quote(void);

    void get_token(void);
    void process_token(bool recur = true);

    void token_next_char(void) {
      token.push_back(*ii);

      ++ii;
      while (*ii == '\r' && ii != endii)
        ++ii;
    }

  private:
    typedef void (markup::*handler_t)(void);

    enum {
      token_none_c,
      token_star_c,
      token_underscore_c,
      token_title_c,
      token_newline_c,
      token_2newline_c,
      token_openbracket_c,
      token_closebracket_c,
      token_bullet_c,
      token_hash_c,
      token_code_c,
      token_beginquote_c,
      token_endquote_c,
    };

    std::string token;
    short token_type;
    const std::string &input;
    std::string::const_iterator ii, endii;
    std::string &output;
    unsigned int flags;
    bool isalive;
}; }

markup::markup(const std::string &_input, std::string &_output, unsigned int _flags)
  : input(_input), output(_output), flags(_flags)
{
  token.reserve(8);
  token_type = token_none_c;

  ii = input.begin();
  endii = input.end();
  isalive = true;

  while (*ii == '\r' && ii != endii)
    ++ii;
}

void markup::do_para(void)
{
  output += "<P>";

  while (isalive) {
    get_token();

    if (token_type == token_2newline_c)
      break;

    process_token();
  }

  output += "</P>";
}

void markup::do_bold(void)
{
  bool morethanone = false;

  while (isalive) {
    get_token();

    if (token_type == token_star_c)
      break;

    if (!morethanone) {
      morethanone = true;
      output += "<B>";
    }
    process_token();
  }

  if (morethanone)
    output += "</B>";
  else
    output += "*";
}

void markup::do_italic(void)
{
  bool morethanone = false;

  while (isalive) {
    get_token();

    if (token_type == token_underscore_c)
      break;

    if (!morethanone) {
      morethanone = true;
      output += "<I>";
    }
    process_token();
  }

  if (morethanone)
    output += "</I>";
  else
    output += "_";
}

void markup::do_link(void)
{
  std::string thelink;
  bool wikilink;

  thelink.reserve(64);

  while (isalive) {
    get_token();

    if (token_type == token_closebracket_c)
      break;

    thelink += token;
  }

  if (thelink.empty())
    return;
  wikilink = (flags & format_wiki_c) && thelink.find('.') == std::string::npos;

  if (wikilink) {
    output += "<A HREF=\"wiki_co.wx?id=";   // not gonna use link_to here, incase they dont actually have the wiki component loaded
    output += urlenc(thelink);
    output += "\">";
    output += html(thelink);
    output += "</A>";
  } else {
    output += "<A HREF=\"";   // not gonna use link_to here, incase they dont actually have the wiki component loaded
    output += html(thelink);
    output += "\">";
    output += html(thelink);
    output += "</A>";
  }
}

void markup::do_list(void)
{
  int level = 1;
  int wantedlevel = 1;

  output += "<UL><LI>";

  while (isalive) {
    get_token();

    if ((token_type != token_none_c && token[0] == '\n') || token_type == token_bullet_c) {
      if (token_type == token_bullet_c)
        wantedlevel = token.size() - 1;   // we do not count the newline in there
      else
        wantedlevel = 0;

      output += "</LI>\n";

      while (level > wantedlevel) {
        output += "</UL>\n";
        --level;
      }
      while (level < wantedlevel) {
        output += "<UL>\n";
        ++level;
      }

      if (wantedlevel == 0)
        return;
      else
        output += "<LI>";
    } else
      process_token();
  }//while

  while (level > 0) {
    output += "</UL>\n";
    --level;
  }
}

void markup::do_numlist(void)
{
  int level = 1;
  int wantedlevel = 1;

  output += "<OL><LI>";

  while (isalive) {
    get_token();

    if ((token_type != token_none_c && token[0] == '\n') || token_type == token_hash_c) {
      if (token_type == token_hash_c)
        wantedlevel = token.size() - 1;   // we do not count the newline in there
      else
        wantedlevel = 0;

      output += "</LI>\n";

      while (level > wantedlevel) {
        output += "</OL>\n";
        --level;
      }
      while (level < wantedlevel) {
        output += "<OL>\n";
        ++level;
      }

      if (wantedlevel == 0)
        return;
      else
        output += "<LI>";
    } else
      process_token();
  }//while

  while (level > 0) {
    output += "</OL>\n";
    --level;
  }
}

void markup::do_title(void)
{
  short level;

  level = token.size();

  if (level < 2)
    level = 2;
  if (level > 4)
    level = 4;

  output += "<H" + int_to_string(level) + ">";

  while (isalive) {
    get_token();

    if (token_type == token_title_c)
      break;

    process_token(false);
  }

  output += "</H" + int_to_string(level) + ">";
}

void markup::do_codequote(void)
{
  output += "<PRE>\n";
  process_token(false);
  while (isalive) {
    get_token();

    if ((token_type != token_none_c && token[0] == '\n') && token_type != token_code_c)
      break;

    process_token(false);
  }
  output += "\n</PRE>\n";
}

void markup::do_quote(void)
{
  output += "<BLOCKQUOTE><P>\n";

  while (isalive) {
    get_token();

    if (token_type == token_endquote_c)
      break;
    if (token_type == token_2newline_c)
      output += "</P><P>\n";
    else
      process_token();
  }

  output += "</P></BLOCKQUOTE>\n";
}

void markup::get_token(void)
{
  token.clear();
  char readchar;

  if (ii == endii) {
    isalive = false;
    return;
  }

  readchar = *ii;
  token_next_char();

  // readchar == current char, *ii is now the look-ahead char

  switch (readchar) {
    case '*': token_type = token_star_c; return;
    case '_': token_type = token_underscore_c; return;
    case '[': token_type = token_openbracket_c; return;
    case ']': token_type = token_closebracket_c; return;
    case '\n':
      if (ii != endii) {
        // only certain \nX combos are tokens
        switch (*ii) {
          case '\n':
            //token_next_char();    // dont ask, but i dont actually consume the 2nd \n in a \n pair
            token_type = token_2newline_c;
            return;
          case '*':
            while (*ii == '*')
              token_next_char();
            token_type = token_bullet_c;
            return;
          case '#':
            while (*ii == '#')
              token_next_char();
            token_type = token_hash_c;
            return;
          case ' ':
            token_next_char();
            token_type = token_code_c;
            return;
          case 'b':
            if (input.substr(ii-input.begin(), 10) == "beginquote") {
              token += input.substr(ii-input.begin(), 10);
              ii += 10;
              token_type = token_beginquote_c;
              return;
            }
            break;
          case 'e':
            if (input.substr(ii-input.begin(), 8) == "endquote") {
              token += input.substr(ii-input.begin(), 8);
              ii += 8;
              token_type = token_endquote_c;
              return;
            }
            break;
        }
      }
      token_type = token_newline_c;
      return;
    case '=':
      while (*ii == '=' && ii != endii)
        token_next_char();
      token_type = token_title_c;
      return;
  }//switch

  token_type = token_none_c;
}

void markup::process_token(bool recur)
{
  // check for new commands

  if (recur)
    switch (token_type) {
      case token_star_c:
        if (flags & format_bold_italics_c) {
          do_bold();
          return;
        }
        break;
      case token_underscore_c:
        if (flags & format_bold_italics_c) {
          do_italic();
          return;
        }
        break;
      case token_openbracket_c:
        if (flags & (format_links_c|format_wiki_c)) {
          do_link();
          return;
        }
        break;
      case token_bullet_c:
        if (flags & format_lists_c) {
          do_list();
          return;
        }
        break;
      case token_hash_c:
        if (flags & format_lists_c) {
          do_numlist();
          return;
        }
        break;
      case token_title_c:
        if (flags & format_titles_c) {
          do_title();
          return;
        }
        break;
      case token_code_c:
        if (flags & format_codequotes_c) {
          do_codequote();
          return;
        }
        break;
      case token_beginquote_c:
        if (flags & format_quotes_c) {
          do_quote();
          return;
        }
        break;
    }

  // ok then, just render the token then
  for (int x=0; x<token.size(); ++x) {
    switch (token[x])
    {
        case '"': output += "&quot;"; break;
        case '&': output += "&amp;"; break;
        case '<': output += "&lt;"; break;
        case '>': output += "&gt;"; break;
        default: output += token[x];
    }
  }
}

void markup::go(void)
{
  while (isalive)
    do_para();
}

std::string wexus::turbo::format_markup(const std::string &input, unsigned int flags)
{
  std::string ret;

  ret.reserve(input.size()*2);

  markup M(input, ret, flags);
  M.go();

  return ret;
}

