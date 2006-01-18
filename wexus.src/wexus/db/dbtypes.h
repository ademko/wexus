
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

#ifndef __INCLUDED_WEXUS_DB_DBTYPES_HPP__
#define __INCLUDED_WEXUS_DB_DBTYPES_HPP__

#include <scopira/tool/platform.h>

#ifdef PLATFORM_win32
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>

#include <string>

#include <scopira/tool/flow.h>
#include <scopira/tool/util.h>
#include <scopira/tool/time.h>

namespace scopira
{
  namespace tool
  {
    // fwd
    class oflow_i;
    class time;
  }
}

namespace wexus
{
  namespace db
  {
    class handle;
    class error;

    class statement; //fwd

    // basic typedefs
    typedef SQLINTEGER dbint;
    typedef SQLDOUBLE dbdouble;

    class dbstring;

    class dbdate;
    class dbtimestamp;

    /// easy convertion
#ifdef PLATFORM_64
    inline bool string_to_dbint(const std::string &s, dbint &out) { return scopira::tool::string_to_int(s, out); }
    inline bool read_dbint(scopira::tool::itflow_i &f, dbint &i) { return f.read_int(i); }
    inline void write_dbint(scopira::tool::otflow_i &f, const dbint &i) { return f.write_int(i); }
#else
    inline bool string_to_dbint(const std::string &s, dbint &out) { return scopira::tool::string_to_long(s, out); }
    inline bool read_dbint(scopira::tool::itflow_i &f, dbint &i) { return f.read_long(i); }
    inline void write_dbint(scopira::tool::otflow_i &f, const dbint &i) { return f.write_long(i); }
#endif

    /// today!
    dbdate today(void);
    /// now!
    dbtimestamp now(void);
  }
}

/**
 * ODBC base handle base class
 *
 * @author Aleksander Demko
 */
class wexus::db::handle : public scopira::tool::object
{
  protected:
    SQLHANDLE m_handle;

    /// gets the handle type
    virtual char get_type(void) const = 0;

    friend class error;
};

/**
 * an error buffer
 *
 * @author Aleksander Demko
 */ 
class wexus::db::error
{
  public:
    /// ctor
    error(void);
    /// ctor
    error(handle &h) { set(h); }

    /// set this error to the current state of the given handle (returns true if has more)
    bool set(handle &h, short recnum = 1);

    /// prints to stream
    void print(scopira::tool::oflow_i &out) const;

    /// prints all the records
    static void print_all(handle &h, scopira::tool::oflow_i &out);

  private:
    enum { len_c = 130 };

    SQLCHAR m_state[6];
    SQLINTEGER m_native;
    SQLCHAR m_msg[len_c];

};

inline scopira::tool::oflow_i & operator << (scopira::tool::oflow_i &o, const wexus::db::error &er)
{
  er.print(o);
  return o;
}

/**
 * A string/char buf container, suitable for binding.
 * This is a special string class, as it has a fixed internal buffer
 * that cannot be resized after contruction.
 *
 * @author Aleksander Demko
 */ 
class wexus::db::dbstring
{
  public:
    /// ctor
    dbstring(const dbstring &s);
    dbstring(const char* s);
    dbstring(const std::string& s);
    explicit dbstring(size_t len = 256);
    /// dtor
    ~dbstring();

    bool load(scopira::tool::iobjflow_i& in);
    void save(scopira::tool::oobjflow_i& out) const;

    /// sets from dbstring
    void set(const dbstring &s);
    /// set from char*
    void set(const char *s);
    /// set from string
    void set(const std::string &s);
    /// set from an int
    void set_int(int x);
    /// sets from a double
    void set_double(double x);
    /// sets null/empty string
    void clear(void);

    dbstring & operator = (const dbstring &s) { set(s); return *this; }
    dbstring & operator = (const std::string &s) { set(s); return *this; }
    dbstring & operator = (const char *s) { set(s); return *this; }

    /// as c string
    char* c_str(void) { return reinterpret_cast<char*>(m_buf); }
    /// as c string
    const char* c_str(void) const { return reinterpret_cast<char*>(m_buf); }

    /// as std::string
    void str(std::string &out) const;
    /// similar
    std::string str(void) const;
    /// implicit converter
    operator std::string(void) const { return str(); }

    /// cur size (does NOT include null)
    size_t size(void) const { return m_cur_size; }
    /// max size (includes null)
    size_t max_size(void) const { return m_max_size; }

    /// is the string empty
    bool empty(void) const { return m_cur_size == 0; }

    bool operator< (const dbstring& rhs) const { return strcmp(c_str(), rhs.c_str()) < 0; }
    bool operator<=(const dbstring& rhs) const { int res = strcmp(c_str(), rhs.c_str()); return (res < 0 || res == 0); }
    bool operator> (const dbstring& rhs) const { return strcmp(c_str(), rhs.c_str()) > 0; }
    bool operator>=(const dbstring& rhs) const { int res = strcmp(c_str(), rhs.c_str()); return (res > 0 || res == 0); }
    bool operator==(const dbstring& rhs) const { return strcmp(c_str(), rhs.c_str()) == 0; }
    bool operator!=(const dbstring& rhs) const { return strcmp(c_str(), rhs.c_str()) != 0; }

    friend class statement;

  private:
    void init(int len);

    SQLCHAR* m_buf;         // the string buffer, null terminated
    SQLINTEGER m_max_size;  // the size of the buffer, total
    SQLINTEGER m_cur_size;  // the size of the string, NOT including the null
};

/**
 * a sql date.
 *  year is year, like 2002
 *  month is month, 1..12
 *  day is day of month, like 1..31 or 1..30 etc
 *
 * @author Aleksander Demko
 */
class wexus::db::dbdate : private DATE_STRUCT
{
  public:
    /// doesnt init to anything!
    dbdate(void);

    /// initing ctor
    dbdate(SQLSMALLINT y, SQLUSMALLINT m, SQLUSMALLINT d);

    /// initing ctor via time
    dbdate(const scopira::tool::timestamp& t);

    /// bulk set
    void set(SQLSMALLINT y, SQLUSMALLINT m, SQLUSMALLINT d);
    /// sets to 0 0 0
    void clear(void);

    /// setter (maybe adding range checking?)
    void set_year(SQLSMALLINT y) { year = y; }
    /// setter
    void set_month(SQLUSMALLINT m) { month = m; }
    /// setter
    void set_day(SQLUSMALLINT d) { day = d; }

    /// getter
    SQLSMALLINT get_year(void) const { return year; }
    /// getter
    SQLUSMALLINT get_month(void) const { return month; }
    /// getter
    SQLUSMALLINT get_day(void) const { return day; }

    /// get a time object equivalent
    scopira::tool::timestamp get_time(void) const;
};

/**
 * a sql time stamp
 *
 * @author Aleksander Demko
 */
class wexus::db::dbtimestamp : private TIMESTAMP_STRUCT
{
  public:
    // doesnt init anything!
    dbtimestamp(void);

    /// ctor initialized with a time object
    dbtimestamp(const scopira::tool::timestamp&);

    /// setter
    void set_year(SQLSMALLINT v) { year = v; }
    /// setter
    void set_month(SQLUSMALLINT v) { month = v; }
    /// setter
    void set_day(SQLUSMALLINT v) { day = v; }
    /// setter
    void set_hour(SQLUSMALLINT v) { hour = v; }
    /// setter
    void set_minute(SQLUSMALLINT v) { minute = v; }
    /// setter
    void set_second(SQLUSMALLINT v) { second = v; }
    /// setter
    void set_fraction(SQLUINTEGER v) { fraction = v; }

    /// getter
    SQLSMALLINT get_year(void) const { return year; }
    /// getter
    SQLUSMALLINT get_month(void) const { return month; }
    /// getter
    SQLUSMALLINT get_day(void) const { return day; }
    /// getter
    SQLUSMALLINT get_hour(void) const { return hour; }
    /// getter
    SQLUSMALLINT get_minute(void) const { return minute; }
    /// getter
    SQLUSMALLINT get_second(void) const { return second; }
    /// getter
    SQLUINTEGER get_fraction(void) const { return fraction; }

    /// get a time object equivalent
    scopira::tool::timestamp get_time(void) const;

    /// sets to "zero"
    void clear(void);
};

scopira::tool::oflow_i& operator<<(scopira::tool::oflow_i& o, wexus::db::dbdate& v);
inline scopira::tool::oflow_i& operator<<(scopira::tool::oflow_i& o, wexus::db::dbstring& v) { return o << v.c_str(); }

#endif

