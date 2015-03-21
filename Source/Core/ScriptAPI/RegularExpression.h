#include <Core/3rdpart/pcreplusplus.h>
#include <string>
#ifdef UNICODE
#undef UNICODE // We do not want to compile sqplus with unicode support
#undef _UNICODE
#include <sqplus.h>
#define UNICODE
#define _UNICODE
#else
#include <sqplus.h>
#endif
namespace ScriptAPI {


class RegularExpression : public pcrepp::Pcre {
public:
	//  Pcre(const std::string& expression);
	RegularExpression(); 
	RegularExpression(const RegularExpression& r);
	RegularExpression(const std::string& expression, const std::string& flags);
	  SquirrelObject split(const std::string& piece);
	  SquirrelObject split_with_limit(const std::string& piece, int limit);
	  SquirrelObject split_with_limit_offset(const std::string& piece, int limit, int start_offset);
	  SquirrelObject split_with_limit_start_end_offset(const std::string& piece, int limit, int start_offset, int end_offset);
	  const std::string get_match(int pos) const;
	  bool search(const std::string& stuff);
	  bool search_with_offset(const std::string& stuff, int OffSet);
	  const std::string replace(const std::string& piece, const std::string& with);
	  int get_entire_match_start() const;
	  int get_match_start(int pos) const;
	  int get_entire_match_end() const;
	  int get_match_end(int pos) const;
	  bool matched();
	  int  matches();
	  SquirrelObject get_sub_strings();

};

RegularExpression* CreateRegExp(const std::string& expression, const std::string& flags);

}