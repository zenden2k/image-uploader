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


#include "pcreplusplus.h"

using namespace std;
using namespace pcrepp;

/*
 * get_*() methods which return (sub)informations such as matches
 * or strings
 */


vector<string>* Pcre::get_sub_strings() const {
  if(resultset != NULL)
    return resultset;
  else
    return NULL;
}

string Pcre::get_match(int pos) const {
  if(pos >= 0 && pos < num_matches) {
    vector<string>::iterator P = resultset->begin() + pos;
    return *P;
  }
  else {
    throw exception("Pcre::get_match(int): out of range");
  }
}

int Pcre::get_match_start() const {
  if (sub_vec)
    return sub_vec[0];
  else
    return -1;
}

int Pcre::get_match_end() const {
  if (sub_vec)
    return sub_vec[1] - 1;
  else
    return -1;
}

int Pcre::get_match_start(int pos) const {
  if(pos >= 0 && pos <= num_matches) {
    /*
     * sub_vec[0] and [1] is the start/end of the entire string.
     */
    return sub_vec[ (++pos) * 2 ];
  }
  else {
    throw exception("Pcre::get_match_start(int): out of range");
  }  
}

int Pcre::get_match_end(int pos) const {
  if(pos >= 0 && pos <= num_matches) {
    /*
     * the end offset of a subpattern points to
     * the first offset of the next substring,
     * therefore -1
     */
    return sub_vec[ ((++pos) * 2) + 1 ] - 1;
  }
  else {
    throw exception("Pcre::get_match_end(int): out of range");
  }
}

size_t Pcre::get_match_length(int pos) const {
  if(pos >= 0 && pos < num_matches) {
    vector<string>::iterator P = resultset->begin() + pos;
    return P->length();
  }
  else {
    throw exception("Pcre::get_match_length(int): out of range");
  }
}


using namespace std;
using namespace pcrepp;

/*
 * replace method
 */

string Pcre::replace(const string& piece, const string& with) {
  /*
   * Pcre::replace version by "Marcus Kramer" <marcus.kramer@scherrer.de>
   */
  string Replaced(piece);

  bool bReplaced = false;
  int  iReplaced = -1;

  __pcredebug << "replace: " << piece << " with: " << with << endl;

  /*
   * certainly we need an anchor, we want to check if the whole arg is in brackets
   * //Pcre braces("^[^\\\\]\\(.*[^\\\\]\\)$"); // perlish: [^\\]\(.*[^\\]\)
   *
   * There's no reason, not to add brackets in general.
   * It's more comfortable, cause we wants to start with $1 at all, 
   * also if we set the whole arg in brackets!
   */
  
  /* recreate the p_pcre* objects to avoid memory leaks */
  pcre_free(p_pcre);
  pcre_free(p_pcre_extra);
  
  pcre       *_p = NULL;
  pcre_extra *_e = NULL;;
        
  p_pcre = _p;
  p_pcre_extra = _e;
  
  if (! _have_paren ) {
    string::size_type p_open, p_close;
    p_open  = _expression.find_first_of("(");
    p_close = _expression.find_first_of(")");
    if ( p_open == string::npos && p_close == string::npos ) {
      /*
       * Well, _expression completely lacks of parens, which are
       * required for search/replace operation. So add parens automatically.
       * Note, that we add 2 pairs of parens so that we finally have $0 matchin
       * the whole expression and $1 matching the inner side (which is in this
       * case the very same string.
       * We do this for perl compatibility. If the expression already contains
       * parens, the whole match will produce $0 for us, so in this case we
       * have no problem
       */
      _expression = "((" + _expression + "))";
    }
    else {
      /*
       * Add parens to the very beginning and end of the expression
       * so that we have $0. I don't care if the user already supplied
       * double-parentesed experssion (e.g. "((1))"), because PCRE seems
       * to eat redundant parenteses, e.g. "((((1))))" returns the same
       * result as "((1))".
       */
        _expression = "(" + _expression;
        _expression=_expression + ")"; 
    }

    _have_paren = true;
  }

  __pcredebug << "_expression: " << _expression << endl;

  Compile(_flags);
        
  if(search(piece)) {
    /* we found at least one match */
    
    // sure we must resolve $1 for ever piece we found especially for "g"
    // so let's just create that var, we resolve it when we needed!
    string use_with;


    if(!global_t) {
      // here we can resolve vars if option g is not set
      use_with = _replace_vars(with);

      if(matched() && matches() >= 1) {
    __pcredebug << "matches: " << matches() << endl;
    int len = get_match_end() - get_match_start() + 1;
    Replaced.replace(get_match_start(0), len, use_with);
    bReplaced  = true;
    iReplaced = 0;
      }
    }
    else {
      /* global replace */

      // in global replace we just need to remember our position
      // so let's initialize it first
      int match_pos = 0;
      while( search( Replaced, match_pos ) ) {
    int len = 0;
                                
    // here we need to resolve the vars certainly for every hit.
    // could be different content sometimes!
    use_with = _replace_vars(with);
                                
    len = get_match_end() - get_match_start() + 1;
    Replaced.replace(get_match_start(0), len, use_with);
                                
    //# Next run should begin after the last char of the stuff we put in the text
    match_pos = ( use_with.length() - len ) + get_match_end() + 1;

    bReplaced  = true;
    ++iReplaced;
      }
    }
  }
  
  did_match   = bReplaced;
  num_matches = iReplaced;

  return Replaced;
}





string Pcre::_replace_vars(const string& piece) {
  /*
   * Pcre::_replace_vars version by "Marcus Kramer" <marcus.kramer@scherrer.de>
   */
  string with  = piece;
  Pcre dollar("\\$([0-9]+)");

  while ( dollar.search( with ) ) {
    // let's do some conversion first
    __pcredebug << "Pcre::dollar matched: " << piece << ". Match(0): " << dollar.get_match(0) << endl;
    int iBracketIndex = atoi( dollar.get_match(1).data() );
    string sBracketContent = get_match(iBracketIndex+1);
    
    // now we can splitt the stuff
    string sSubSplit = "\\$" + dollar.get_match(1);
    Pcre subsplit(sSubSplit);
                
    // normally 2 (or more) parts, the one in front of and the other one after "$1"
    vector<string> splitted = subsplit.split(with); 
    string Replaced;
                
    for(size_t pos=0; pos < splitted.size(); pos++) {
      if( pos == ( splitted.size() - 1 ) ) 
    Replaced += splitted[pos];
      else 
    Replaced += splitted[pos] + sBracketContent;
    }
    with = Replaced; // well, one part is done
  }
  return with;
}


/*
 * the search interface to pcre
 */


/*
 * compile the expression
 */
void Pcre::Compile(int flags) {
  p_pcre       = pcre_compile((char *)_expression.c_str(), flags,
                  (const char **)(&err_str), &erroffset, tables);

  if(p_pcre == NULL) {
    /* umh, that's odd, the parser should not fail at all */
    string Error = err_str;
    throw exception("pcre_compile(..) failed: " + Error + " at: " + _expression.substr(erroffset));
  }

  /* calculate the number of substrings we are willing to catch */
  int where;
  int info = pcre_fullinfo( p_pcre, p_pcre_extra, PCRE_INFO_CAPTURECOUNT, &where);
  if(info == 0) {
    sub_len = (where +2) * 3; /* see "man pcre" for the exact formula */
  }
  else {
    throw exception(info);
  }
  reset();
}




/*
 * API methods
 */
bool Pcre::search(const string& stuff, int OffSet){
  return dosearch(stuff, OffSet);
}

bool Pcre::search(const string& stuff){
  return dosearch(stuff, 0);
}

bool Pcre::dosearch(const string& stuff, int OffSet){
  reset();
  if (sub_vec != NULL)
    delete[] sub_vec;

  sub_vec = new int[sub_len];
  int num = pcre_exec(p_pcre, p_pcre_extra, (char *)stuff.c_str(),
                        (int)stuff.length(), OffSet, 0, (int *)sub_vec, sub_len);

  __pcredebug << "Pcre::dosearch(): pcre_exec() returned: " << num << endl;

  if(num < 0) {
    /* no match at all */
    __pcredebug << " - no match" << endl;
    return false;
  }
  else if(num == 0) {
    /* vector too small, there were too many substrings in stuff */
    __pcredebug << " - too many substrings" << endl;
    return false;
  }
  else if(num == 1) {
    /* we had a match, but without substrings */
    __pcredebug << " - match without substrings" << endl;
    did_match = true;
    num_matches = 0;
    return true;
  }
  else if(num > 1) {
    /* we had matching substrings */
    if (resultset != NULL)
      delete resultset;
    resultset = new vector<string>;
    const char **stringlist;
    did_match = true;
    num_matches = num /*- 1*/;

    __pcredebug << " - match with " << num_matches << " substrings" << endl;

    int res = pcre_get_substring_list((char *)stuff.c_str(), sub_vec, num, &stringlist);
    if(res == 0) {
      __pcredebug << "Pcre::dosearch(): matched substrings: " << endl;
      for(int i=0; i<num; i++) {
    __pcredebug << " " << string(stringlist[i]) << endl;
    resultset->push_back(stringlist[i]);
      }
      pcre_free_substring_list(stringlist);
    }
    else {
      throw exception(res);
    }
    return true;
  }
  else {
    /* some other uncommon error occured */
    __pcredebug << " - uncommon error" << endl;
    return false;
  }
}


vector<string> Pcre::_split(const string& piece, int limit, int start_offset, int end_offset) {
  vector<string> Splitted;
  /* _expression will be used as delimiter */
  if(_expression.length() == 1) {
    /* use the plain c++ way, ignore the pre-compiled p_pcre */
    string buffer, _delimiter, _piece;
    char z;
    if(case_t) {
      z = toupper(_expression[0]);
      for(size_t pos=0; pos < piece.length(); pos++) {
    _piece += (char)toupper(piece[pos]);
      }
    }
    else {
      z = _expression[0];
      _piece = piece;
    }
    for(size_t pos=0; pos<piece.length(); pos++) {
      if(_piece[pos] == z) {
    Splitted.push_back(buffer);
    buffer = "";
      }
      else {
    buffer += piece[pos];
      }
    }
    if(buffer != "") {
      Splitted.push_back(buffer);
    }
  }
  else {
    /* use the regex way */
    if(_expression[0] != '(' && _expression[ _expression.length() - 1 ] != ')' ) {
      /* oh, oh - the pre-compiled expression does not contain brackets */
      pcre_free(p_pcre);
      pcre_free(p_pcre_extra);
      
      pcre       *_p = NULL;
      pcre_extra *_e = NULL;;

      p_pcre = _p;
      p_pcre_extra = _e;

      _expression = "(" + _expression + ")";
      Compile(_flags);
    }
    int num_pieces=0, pos=0, piece_end = 0, piece_start = 0;
    for(;;) {
      if(search(piece, pos) == true) {
    if(matches() > 0) {
      piece_end   = get_match_start(0) - 1;
      piece_start = pos;
      pos = piece_end + 1 + get_match_length(0);
      string junk(piece, piece_start, (piece_end - piece_start)+1);
      num_pieces++;
      if( (limit != 0 && num_pieces < limit) || limit == 0) {
        if( (start_offset != 0 && num_pieces >= start_offset) || start_offset == 0) {
          if( (end_offset != 0 && num_pieces <= end_offset) || end_offset == 0) {
        /* we are within the allowed range, so just add the grab */
        Splitted.push_back(junk);
          }
        }
      }
    }
      }
      else {
    /* the rest of the string, there are no more delimiters */
    string junk(piece, pos, (piece.length() - pos));
    num_pieces++;
    if( (limit != 0 && num_pieces < limit) || limit == 0) {
      if( (start_offset != 0 && num_pieces >= start_offset) || start_offset == 0) {
        if( (end_offset != 0 && num_pieces <= end_offset) || end_offset == 0) {
          /* we are within the allowed range, so just add the grab */
          Splitted.push_back(junk);
        }
      }
    }
    break;
      }
    } // for()
  } // if(_expression.length()
  return Splitted;
}

vector<string> Pcre::split(const string& piece) {
  return _split(piece, 0, 0, 0);
}

vector<string> Pcre::split(const string& piece, int limit) {
  return _split(piece, limit, 0, 0);
}

vector<string> Pcre::split(const string& piece, int limit, int start_offset) {
  return _split(piece, limit, start_offset, 0);
}

vector<string> Pcre::split(const string& piece, int limit, int start_offset, int end_offset) {
  return _split(piece, limit, start_offset, end_offset);
}

vector<string> Pcre::split(const string& piece, vector<int> positions) {
  vector<string> PreSplitted = _split(piece, 0, 0, 0);
  vector<string> Splitted;
  for(vector<int>::iterator vecIt=positions.begin(); vecIt != positions.end(); ++vecIt) {
    Splitted.push_back(PreSplitted[*vecIt]);
  }
  return Splitted;
}


/*
 * CONSTRUCTORS and stuff
 */
Pcre::Pcre(const string& expression) {
  _have_paren    = false;
  _expression   = expression;
  _flags        = 0;
  case_t = global_t = false;
  zero();
  Compile(0);
}

Pcre::Pcre(const string& expression, const string& flags) {
  _have_paren    = false;
  _expression   = expression;
  unsigned int FLAG = 0;

  for(unsigned int flag=0; flag<flags.length(); flag++) {
    switch(flags[flag]) {
    case 'i': FLAG |= PCRE_CASELESS;  case_t = true;   break;
    case 'm': FLAG |= PCRE_MULTILINE;                  break;
    case 's': FLAG |= PCRE_DOTALL;                     break;
    case 'x': FLAG |= PCRE_EXTENDED;                   break;
#ifdef PCRE_UCP
    case 'u': FLAG |= PCRE_UTF8|PCRE_UCP;                        break;
#else 
    case 'u': FLAG |= PCRE_UTF8;                        break;
#endif
    case 'g':                         global_t = true; break;
    }
  }

  _flags = FLAG;

  zero();
  Compile(FLAG);
}

Pcre::Pcre(const string& expression, unsigned int flags) {
  _have_paren    = false;
  _expression = expression;
  _flags = flags;

  if((_flags & PCRE_CASELESS) != 0)
    case_t = true;

  if((_flags & PCRE_GLOBAL) != 0) {
    global_t = true;
    _flags = _flags - PCRE_GLOBAL; /* remove pcre++ flag before feeding _flags to pcre */
  }

  zero();
  Compile(_flags);
}

Pcre::Pcre(const Pcre &P) {
  _have_paren = P._have_paren;
  _expression = P._expression;
  _flags      = P._flags;
  case_t      = P.case_t;
  global_t    = P.global_t;
  zero();
  Compile(_flags);
}

Pcre::Pcre() {
  zero();
}







/*
 * Destructor
 */
Pcre::~Pcre() {
  /* avoid deleting of uninitialized pointers */
  if (p_pcre != NULL) {
    pcre_free(p_pcre);
  }
  if (p_pcre_extra != NULL) {
    pcre_free(p_pcre_extra);
  }
  if(sub_vec != NULL) {
    delete[] sub_vec;
  }
  if(resultset != NULL) {
    delete resultset;
  }
}




/*
 * operator= definitions
 */
const Pcre& Pcre::operator = (const string& expression) {
  /* reset the object and re-intialize it */
  reset();
  _expression = expression;
  _flags      = 0;
  case_t = global_t = false;
  Compile(0);
  return *this;
}


const Pcre& Pcre::operator = (const Pcre &P) {
  reset();
  _expression = P._expression;
  _flags      = P._flags;
  case_t      = P.case_t;
  global_t    = P.global_t;
  zero();
  Compile(_flags);
  return *this;
}






/*
 * mem resetting methods
 */
void Pcre::zero() {
  /* what happens if p_pcre is already allocated? hm ... */
  p_pcre_extra = NULL;
  p_pcre       = NULL;
  sub_vec      = NULL;
  resultset    = NULL;
  err_str      = NULL;
  num_matches  = -1;
  tables       = NULL;
}

void Pcre::reset() {
  did_match   = false;
  num_matches = -1;
}





/*
 * accessing the underlying implementation
 */
pcre* Pcre::get_pcre() {
  return p_pcre;
}

pcre_extra* Pcre::get_pcre_extra() {
  return p_pcre_extra;
}





/*
 * support stuff
 */
void Pcre::study() {
  p_pcre_extra = pcre_study(p_pcre, 0, (const char **)(&err_str));
  if(err_str != NULL)
    throw exception("pcre_study(..) failed: " + string(err_str));
}


/*
 * setting current locale
 */
bool Pcre::setlocale(const char* locale) {
 /* if (std::setlocale(LC_CTYPE, locale) == NULL) return false;
  tables = pcre_maketables();*/
  return true;
}
