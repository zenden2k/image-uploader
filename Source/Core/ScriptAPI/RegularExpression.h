#include <Core/3rdpart/pcreplusplus.h>
#include <string>
#include "../Squirrelnc.h"
#include <Core/Utils/CoreTypes.h>
namespace ScriptAPI {


class RegularExpression  {
public:
	//  Pcre(const std::string& expression);
	RegularExpression(); 
	//RegularExpression(const RegularExpression& r);
	RegularExpression(const std::string& expression, const std::string& flags);
	  SquirrelObject split(const std::string& piece);
	  SquirrelObject splitWithLimit(const std::string& piece, int limit);
	  SquirrelObject splitWithLimitOffset(const std::string& piece, int limit, int start_offset);
	  SquirrelObject splitWithLimitStartEndOffset(const std::string& piece, int limit, int start_offset, int end_offset);

	  const std::string getMatch(int pos) const;
	  bool search(const std::string& stuff);
	  bool searchWithOffset(const std::string& stuff, int OffSet);
	  SquirrelObject findAll(const std::string& stuff);
	  const std::string replace(const std::string& piece, const std::string& with);
	  int getEntireMatchStart() const;
	  int getMatchStart(int pos) const;
	  int getEntireMatchEnd() const;
	  int getMatchEnd(int pos) const;
	  bool match(const std::string& stuff);
	  bool matched();
	  int  matchesCount();
	  SquirrelObject getSubStrings();
protected:
	std_tr::shared_ptr<pcrepp::Pcre> pcre_;

};

RegularExpression CreateRegExp(const std::string& expression, const std::string& flags = "");
void RegisterRegularExpressionClass();
}