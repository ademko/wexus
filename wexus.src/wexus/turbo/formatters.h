
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

#ifndef __INCLUDED_WEXUS_TURBO_FORMATS_H__
#define __INCLUDED_WEXUS_TURBO_FORMATS_H__

#include <string>
#include <wexus/core/http.h>

namespace wexus
{
  namespace turbo
  {
    /// html encoder
    inline std::string html(const std::string &s) { return wexus::core::html_quote_encode(s); }
    inline std::string urlenc(const std::string &s) { return wexus::core::url_encode(s); }

    enum {
      format_links_c = 1,   // normal links
      format_wikilinks_c = 2,
      format_bold_italics_c = 4,
      format_titles_c = 8,   // headings/titles
      format_lists_c = 16,    // numeric and non-numeric lists
      format_quotes_c = 32,   // post quoting and code quote
      format_codequotes_c = 64,   // newline+space == fixed width quotes

      format_basic_c = 0,      // html encoding and paragraph coding is ALWAYS ON
      format_post_c = format_links_c|format_bold_italics_c|format_quotes_c,
      format_wiki_c = 0x7F, // ALL flags
    };

    /**
     * Format the given string as user markup.
     *
     * As flags, either choose format_basic_c, format_post_c or format_wiki_c, or if you
     * want total controler, the indiviual type flags.
     *
     * @author Aleksander Demko
     */ 
    std::string format_markup(const std::string &input, unsigned int flags = format_basic_c);
  }
}

#endif

