/*
 *
 *  This file  is part of the PCRE++ Class Library.
 *
 *  By  accessing  this software,  PCRE++, you  are  duly informed
 *  of and agree to be  bound  by the  conditions  described below
 *  in this notice:
 *
 *  This software product,  PCRE++,  is developed by Thomas Linden
 *  and copyrighted (C) 2002-2003 by Thomas Linden,with all rights 
 *  reserved.
 *
 *  There  is no charge for PCRE++ software.  You can redistribute
 *  it and/or modify it under the terms of the GNU  Lesser General
 *  Public License, which is incorporated by reference herein.
 *
 *  PCRE++ is distributed WITHOUT ANY WARRANTY, IMPLIED OR EXPRESS,
 *  OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE or that
 *  the use of it will not infringe on any third party's intellec-
 *  tual property rights.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with PCRE++.  Copies can also be obtained from:
 *
 *    http://www.gnu.org/licenses/lgpl.txt
 *
 *  or by writing to:
 *
 *  Free Software Foundation, Inc.
 *  59 Temple Place, Suite 330
 *  Boston, MA 02111-1307
 *  USA
 *
 *  Or contact:
 *
 *   "Thomas Linden" <tom@daemon.de>
 *
 *
 */

#ifndef HAVE_PCRE_PP_H
#define HAVE_PCRE_PP_H

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>
#include <iostream>


extern "C" {
  #include <pcre.h>
  #include <locale.h>
}

namespace pcrepp {

#ifdef DEBUG
#define __pcredebug cerr << "(pcre++ DEBUG) " << __LINE__ << ": " 
#else
#define __pcredebug if(0) cerr 
#endif

/** additional binary flag in the hope Philip never ever uses the same in the future
 * @see Pcre(const std::string& expression, unsigned int flags)
 */
#define PCRE_GLOBAL 0x10000


/**
 * The Pcre class is a wrapper around the PCRE library.
 *
 * The library "pcre++" defines a class named "Pcre" which you
 * can use to search in strings using reular expressions as
 * well as getting matched sub strings. It does currently not
 * support all features, which the underlying PCRE library
 * provides, but the most important stuff is implemented.
 *
 * Please study this example code to learn how to use this class:
 * @include examples/demo.cc
 *
 * Compile your programs which use the prce++ class using the following
 * command line:
 * @code
 *   g++ -c yourcode.o `pcre-config --cflags` `pcre++-config --cflags`
 *   g++ yourcode.o `pcre-config --libs` `pcre++-config --libs` -o yourprogram
 * @endcode
 *
 * If you want to learn more about regular expressions which can be used
 * with pcre++, then please read the following documentation:
 * <a href="perlre.html">perlre - Perl regular expressions</a>
 *
 * The pcre library itself does also contain some usefull documentation,
 * which maybe interesting for you:
 * <a href="pcre.html">PCRE manual page</a>
 */

class Pcre {
 private:
  std::string _expression;   /* the given regular expression */
  unsigned int _flags;       /* the given flags, 0 if not defined */
  bool case_t, global_t;     /* internal compile flags, used by replace() and split() */
  pcre *p_pcre;              /* pcre object pointer */
  pcre_extra *p_pcre_extra;  /* stuff required by pcre lib */
  int sub_len;
  int *sub_vec;
  int erroffset;
  char *err_str;
  std::vector<std::string> *resultset;          /* store substrings, if any */
  bool _have_paren;          /* indicate wether we have already parentesis applied in replace */

  const unsigned char *tables; /* locale tables */

  bool did_match;            /** true if the expression produced a match */
  int  num_matches;          /** number of matches if std::vector<std::string>* expected */

  /* reset all counters and free objects, prepare for another search */
  void reset();

  /* compile the pattern */
  void Compile(int flags);

  /* do the actual search, will be called by the public ::search(..) methods */
  bool dosearch(const std::string& stuff, int OffSet);

  /* do the actual split() job, called by the various wrapper split() methods */
  std::vector<std::string> _split(const std::string& piece, int limit, int start_offset, int end_offset);
  
  /* replace $1 .. $n with the corresponding substring, used by replace() */
  std::string _replace_vars(const std::string& piece);

  /* init pointers with NULL */
  void zero();

  std::map<std::string,std::string> info();
  std::string info(int what);

 public:

  /**
   * Exception wrapper class.
   *
   * All errors which may occur inside the Pcre class will
   * throw an exception of this type("Pcre::exception").
   *
   * You can catch such exceptions like this:
   *
   * @code
   * try {
   *   ..
   * }
   * catch(Pcre::exception &e) {
   *   cout << "Pcre++ error: " << e.what() << endl;
   * }
   * @endcode
   */
  class exception : public std::runtime_error {
  private:
    std::string translate(int num) {
      std::string msg;
      switch(num) {
      case -1: msg = "PCRE_ERROR_NOMATCH";      break;
      case -2: msg = "PCRE_ERROR_NULL";         break;
      case -3: msg = "PCRE_ERROR_BADOPTION";    break;
      case -4: msg = "PCRE_ERROR_BADMAGIC";     break;
      case -5: msg = "PCRE_ERROR_UNKNOWN_NODE"; break;
      case -6: msg = "PCRE_ERROR_NOMEMORY";     break;
      case -7: msg = "PCRE_ERROR_NOSUBSTRING";  break;
	// pcre4-HINT: add PCRE_ERROR_MATCHLIMIT support
      }
      return msg;
    }
  public:
    exception(const std::string & msg) : runtime_error(msg) { }
    exception(int num) : runtime_error(translate(num)) { }
  };


  /** Empty Constructor.
   * Create a new empty Pcre object. This is the simplest
   * constructor available, you might consider one of the other
   * constructors as a better solution.
   * You need to initialize thie Pcre object, if you use the
   * empty constructor. You can use one of the two available
   * operator= operators to assign it an expression or a Pcre
   * copy.
   *
   * @return A new empty Pcre object
   */
  Pcre();

  /** Constructor.
   * Compile the given pattern. An Pcre object created this way can
   * be used multiple times to do searches.
   *
   * @param "expression"  a string, which must be a valid perl regular expression.
   * @return A new Pcre object, which holds te compiled pattern.
   * @see Pcre(const std::string& expression, const std::string& flags)
   * @see Pcre(const std::string& expression, unsigned int flags)
   */
  Pcre(const std::string& expression);

  /** Constructor.
   * Compile the given pattern. An Pcre object created this way can
   * be used multiple times to do searches.
   *
   * @param "expression"  a string, which must be a valid perl regular expression.
   * @param "flags" can be one or more of the following letters:
   *
   *- <b>i</b>   Search case insensitive.
   *
   *- <b>m</b>   Match on multiple lines, thus ^ and $ are interpreted
   *             as the start and end of the entire string, not of a
   *             single line.
   *
   *- <b>s</b>   A dot in an expression matches newlines too(which is
   *             normally not the case).
   *
   *- <b>x</b>   Whitespace characters will be ignored (except within
   *             character classes or if escaped).
   *
   *- <b>g</b>   Match multiple times. This flags affects only the behavior of the
   *             replace(const std::string& piece, const std::string& with) method.
   *
   * @return A new Pcre object, which holds te compiled pattern.
   * @see Pcre(const std::string& expression)
   * @see Pcre(const std::string& expression, unsigned int flags)
   */
  Pcre(const std::string& expression, const std::string& flags);

  /** Constructor.
   * Compile the given pattern. An Pcre object created this way can
   * be used multiple times to do searches.
   *
   * @param "expression"  a string, which must be a valid perl regular expression.
   * @param "flags" option bits can be one or more of the following bits:
   *
   * - PCRE_ANCHORED        anchored pattern.
   * - PCRE_CASELESS        case insensitive search.
   * - PCRE_DOLLAR_ENDONLY  dollar sign matches only at end.
   * - PCRE_DOTALL          newline is contained in .
   * - PCRE_EXTENDED        whitespace characters will be ignored.
   * - PCRE_EXTRA           use perl incompatible pcre extensions.
   * - PCRE_MULTILINE       match on multiple lines.
   * - PCRE_NO_AUTO_CAPTURE disable the use of numbered capturing parentheses in the pattern. 
   * - PCRE_UNGREEDY        qunatifiers behave not greedy by default.
   * - PCRE_UTF8            use utf8 support.
   * - PCRE_GLOBAL          (PCRE++ internal flag) match multiple times used only in the
   *                        replace(const std::string& piece, const std::string& with) method.
   *
   * @return A new Pcre object, which holds te compiled pattern.
   * @see Pcre(const std::string& expression)
   * @see Pcre(const std::string& expression, const std::string& flags)
   * @see pcreapi(3) manpage
   */
  Pcre(const std::string& expression, unsigned int flags);

  /** Copy Constructor
   * Creates a new Pcre object of an existing one.
   * @param "P" an existing Pcre object.
   * @return A new Pcre object, which holds te compiled pattern.
   * @see Pcre(const std::string& expression)
   * @see Pcre(const std::string& expression, const std::string& flags)
   */
  Pcre(const Pcre &P);

  /** Operator =.
   * @param "expression" a valid regular expression.
   * @return a new Pcre object.
   *
   * Example:
   *
   * @code
   * Pcre regex = "(A+?)";
   * @endcode
   */
  const Pcre& operator = (const std::string& expression); 

  /** Operator =.
   * @param "&P" an Pcre object
   * @return a new Pcre object
   *
   * Example:
   *
   * @code
   * Pcre reg1("^[a-z]+?");
   * Pcre reg2;
   * reg2 = reg1;
   * @endcode
   */
  const Pcre& operator = (const Pcre &P);

  /** Destructor.
   * The desturcor will automatically invoked if the object
   * is no more used. It frees all the memory allocated by
   * pcre++.
   */
  ~Pcre();

  /** Do a search on the given string.
   * This method does the actual search on the given string.
   * @param "stuff" the string in which you want to search for something.
   * @return boolean <b>true</b> if the regular expression matched. <b>false</b> if not.
   * @see bool search(const std::string& stuff, int OffSet)
   */
  bool search(const std::string& stuff);

  /** Do a search on the given string beginning at the given offset.
   * This method does the actual search on the given string.
   * @param "stuff" the string in which you want to search for something.
   * @param "OffSet" the offset where to start the search.
   * @return boolean <b>true</b> if the regular expression matched. <b>false</b> if not.
   * @see bool search(const std::string& stuff)
   */
  bool search(const std::string& stuff, int OffSet);

  /** Return a vector of substrings, if any.
   * @return a pointer to an std::vector<std::string>, which may be NULL, if no substrings has been found.
   * @see std::vector<std::string>
   */
  std::vector<std::string>* get_sub_strings() const;

  /** Get a substring at a known position. 
   * This method throws an out-of-range exception if the given position
   * is invalid.
   * @param "pos" the position of the substring to return. Identical to perl's $1..$n.
   * @return the substring at the given position.
   *
   * Example:
   * @code
   * 
   * std::string mysub = regex.get_match(1); 
   * 
   * @endcode
   * Get the first substring that matched the expression in the "regex" object.
   */
  std::string get_match(int pos) const;

  /** Get the start position of a substring within the searched string.
   * This method returns the character position of the first character of
   * a substring withing the searched string.
   * @param "pos" the position of the substring. Identical to perl's $1..$n.
   * @return the integer character position of the first character of a substring. Positions are starting at 0.
   *
   * Example:
   * @code
   * 
   * Pcre regex("([0-9]+)");               // search for numerical characters
   * regex.search("The 11th september.");  // do the search on this string
   * std::string day = regex.get_match(1);      // returns "11"
   * int pos = regex.get_match_start(1);   // returns 4, because "11" begins at the
   *                                       // 4th character inside the search string.
   *
   * @endcode
   * @see int get_match_end(int pos)
   * @see int get_match_end()
   * @see int get_match_start()
   */
  int get_match_start(int pos) const;

  /** Get the end position of a substring within the searched string.
   * This method returns the character position of the last character of
   * a substring withing the searched string.
   * @param "pos" the position of the substring. Identical to perl's $1..$n.
   * @return the integer character position of the last character of a substring. Positions are starting at 0.
   *
   * Example:
   * @code
   *
   * Pcre regex("([0-9]+)");               // search for numerical characters
   * regex.search("The 11th september.");  // do the search on this string
   * std::string day = regex.get_match(1);      // returns "11"
   * int pos = regex.get_match_end(1);     // returns 5, because "11" ends at the
   *                                       // 5th character inside the search string.
   *
   * @endcode
   * @see int get_match_start(int pos)
   * @see int get_match_start()
   * @see int get_match_end()
   */
  int get_match_end(int pos) const;




  /** Get the start position of the entire match within the searched string.
   * This method returns the character position of the first character of
   * the entire match within the searched string.
   * @return the integer character position of the first character of the entire match.
   *
   * Example:
   * @code
   *
   * Pcre regex("([0-9]+)\s([a-z]+)");     // search for the date(makes 2 substrings
   * regex.search("The 11th september.");  // do the search on this string
   * int pos = regex.get_match_start();    // returns 4, because "11th september" begins at the
   *                                       // 4th character inside the search string.
   *
   * @endcode
   * @see int get_match_start(int pos)
   * @see int get_match_end(int pos)
   * @see int get_match_end()
   */
  int get_match_start() const;

  /** Get the end position of the entire match within the searched string.
   * This method returns the character position of the last character of
   * the entire match within the searched string.
   * @return the integer character position of the last character of the entire match.
   *
   * Example:
   * @code
   *
   * Pcre regex("([0-9]+)\s([a-z]+)");     // search for the date(makes 2 substrings
   * regex.search("The 11th september.");  // do the search on this string
   * int pos = regex.get_match_end();      // returns 17, because "11th september", which is
                                           // the entire match, ends at the
   *                                       // 17th character inside the search string.
   *
   * @endcode
   * @see int get_match_start()
   * @see int get_match_start(int pos)
   * @see int get_match_end(int pos)
   */
  int get_match_end() const;




  /** Get the length of a substring at a known position. 
   * This method throws an out-of-range exception if the given position
   * is invalid.
   * @param "pos" the position of the substring-length to return. Identical to perl's $1..$n.
   * @return the length substring at the given position.
   *
   */
  size_t get_match_length(int pos) const;

  /** Test if a search was successfull.
   * This method must be invoked <b>after</b> calling search().
   * @return boolean <b>true</b> if the search was successfull at all, or <b>false</b> if not.
   */
  bool matched() const { return did_match; };

  /** Get the number of substrings generated by pcre++.
   * @return the number of substrings generated by pcre++.
   */
  int  matches() const { return num_matches; }


  /** split a string into pieces
   * This method will split the given string into a vector
   * of strings using the compiled expression (given to the constructor).
   *
   * @param "piece" The string you want to split into it's parts.
   * @return an vector of strings
   * @see std::vector<std::string>
   * @see std::vector<std::string> split(const std::string& piece, int limit)
   * @see std::vector<std::string> split(const std::string& piece, int limit, int start_offset)
   * @see std::vector<std::string> split(const std::string& piece, int limit, int start_offset, int end_offset)
   * @see std::vector<std::string> split(const std::string& piece, std::vector<int> positions)
   */
  std::vector<std::string> split(const std::string& piece);

  /** split a string into pieces
   * This method will split the given string into a vector
   * of strings using the compiled expression (given to the constructor).
   *
   * @param "piece" The string you want to split into it's parts.
   * @param "limit" the maximum number of elements you want to get back from split().
   * @return an vector of strings
   * @see std::vector<std::string>
   * @see std::vector<std::string> split(const std::string& piece)
   * @see std::vector<std::string> split(const std::string& piece, int limit, int start_offset)
   * @see std::vector<std::string> split(const std::string& piece, int limit, int start_offset, int end_offset)
   * @see std::vector<std::string> split(const std::string& piece, std::vector<int> positions)
   */
  std::vector<std::string> split(const std::string& piece, int limit);

  /** split a string into pieces
   * This method will split the given string into a vector
   * of strings using the compiled expression (given to the constructor).
   *
   * @param "piece" The string you want to split into it's parts.
   * @param "limit" the maximum number of elements you want to get back from split().
   * @param "start_offset" at which substring the returned vector should start.
   * @return an vector of strings
   * @see std::vector<std::string>
   * @see std::vector<std::string> split(const std::string& piece)
   * @see std::vector<std::string> split(const std::string& piece, int limit)
   * @see std::vector<std::string> split(const std::string& piece, int limit, int start_offset, int end_offset)
   * @see std::vector<std::string> split(const std::string& piece, std::vector<int> positions)
   */
  std::vector<std::string> split(const std::string& piece, int limit, int start_offset);

  /** split a string into pieces
   * This method will split the given string into a vector
   * of strings using the compiled expression (given to the constructor).
   *
   * @param "piece" The string you want to split into it's parts.
   * @param "limit" the maximum number of elements you want to get back from split().
   * @param "start_offset" at which substring the returned vector should start.
   * @param "end_offset" at which substring the returned vector should end.
   * @return an vector of strings
   * @see std::vector<std::string> split(const std::string& piece)
   * @see std::vector<std::string> split(const std::string& piece, int limit)
   * @see std::vector<std::string> split(const std::string& piece, int limit, int start_offset)
   * @see std::vector<std::string> split(const std::string& piece, std::vector<int> positions)
   * @see std::vector<std::string>
   */
  std::vector<std::string> split(const std::string& piece, int limit, int start_offset, int end_offset);

  /** split a string into pieces
   * This method will split the given string into a vector
   * of strings using the compiled expression (given to the constructor).
   *
   * @param "piece" The string you want to split into it's parts.
   * @param "positions" a std::vector<int> of positions, which the returned vector should contain.
   * @return an vector of strings
   * @see std::vector<std::string> split(const std::string& piece)
   * @see std::vector<std::string> split(const std::string& piece, int limit)
   * @see std::vector<std::string> split(const std::string& piece, int limit, int start_offset)
   * @see std::vector<std::string> split(const std::string& piece, int limit, int start_offset)
   * @see std::vector<std::string>
   */
  std::vector<std::string> split(const std::string& piece, std::vector<int> positions);

  /** Replace parts of a string using regular expressions.
   * This method is the counterpart of the perl s/// operator.
   * It replaces the substrings which matched the given regular expression
   * (given to the constructor) with the supplied string.
   *
   * @param "piece" the string in which you want to search and replace.
   * @param "with"  the string which you want to place on the positions which match the expression (given to the constructor).
   */
  std::string replace(const std::string& piece, const std::string& with);

  /** Return pointer to underlying pcre object.
   * The pcre object allows you to access the pcre API directly.
   * E.g. if your are using pcre version 4.x and want to use the
   * new functionality which is currently not supported by pcre++.
   * An example would be: pcre_fullinfo(), pcre_study() or the
   * callout functionality.
   *
   * @return "pcre*" pointer to pcre object.
   * @see man pcre
   * @see pcre_extra* get_pcre_extra()
   */
  pcre* get_pcre();

  /** Return pointer to underlying pcre_extra structure.
   * The returned pcre_extra structure can be used in conjunction
   * with the pcre* object returned by pcre().
   *
   * @return "pcre_extra*" pointer to pcre_extra structure.
   * @see pcre* get_pcre()
   */
  pcre_extra* get_pcre_extra();

  /** Analyze pattern for speeding up the matching process.
   * When a pattern is going to be used several times, it  is  worth  spending  more
   * time  analyzing  it in order to speed up the time taken for matching.
   *
   * An excpetion will be thrown if analyzing the pattern failed.
   */
  void study();

  /** Sets locale for all character operations
   * Returns false if locale can't be set. Otherwise returns true
   * @param "locale" locale alias name you want to use.
   * @return true if setting locale were successful
   *
   * @see locale(1)
   */
  bool setlocale(const char* locale);

  /** Return substring of a match at a known possition using the array notation.
   * This method throws an out-of-range exception if the given position
   * is invalid.
   * @param "index" the position of the substring to return. Identical to perl's $1..$n.
   * @return the substring at the given position.
   *
   * Example:
   * @code
   * 
   * std::string mysub = regex[1]; 
   * 
   * @endcode
   * Get the first substring that matched the expression in the "regex" object.
   *
   * @see std::string get_match(int pos)
   */
  std::string operator[](int index) {
    return get_match(index);
  }
}; 

} // end namespace pcre

#endif // HAVE_PCRE_PP_H
