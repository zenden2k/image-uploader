#include "RegularExpression.h"

#include <Core/Logging.h>
#include "../Squirrelnc.h"

using namespace pcrepp;
using namespace ScriptAPI;
DECLARE_INSTANCE_TYPE(RegularExpression);

namespace ScriptAPI {

RegularExpression::RegularExpression(const std::string& expression, const std::string& flags) : Pcre(expression, flags)
{

}

RegularExpression::RegularExpression()
{

}

RegularExpression::RegularExpression(const RegularExpression& r)
{
	Pcre::Pcre(r);
}

SquirrelObject RegularExpression::split(const std::string& piece)
{
	try {
		std::vector<std::string> res = Pcre::split(piece);
		SquirrelObject obj = SquirrelVM::CreateArray(res.size());
		for( int i = 0; i  < res.size(); i++ ) {
			obj.SetValue(i, res[i].c_str());
		}
		return obj;
	} catch( Pcre::exception ex ) {
		LOG(ERROR) << ex.what();
		return SquirrelObject();
	}
}

SquirrelObject RegularExpression::splitWithLimitOffset(const std::string& piece, int limit, int start_offset)
{
	try {
		std::vector<std::string> res = Pcre::split(piece,limit,start_offset);
		SquirrelObject obj = SquirrelVM::CreateArray(res.size());
		for( int i = 0; i  < res.size(); i++ ) {
			obj.SetValue(i, res[i].c_str());
		}
		return obj;
	}
	catch( Pcre::exception ex ) {
		LOG(ERROR) << ex.what();
		return SquirrelObject();
	}
}

SquirrelObject RegularExpression::splitWithLimit(const std::string& piece, int limit)
{
	try {
		std::vector<std::string> res = Pcre::split(piece,limit);
		SquirrelObject obj = SquirrelVM::CreateArray(res.size());
		for( int i = 0; i  < res.size(); i++ ) {
			obj.SetValue(i, res[i].c_str());
		}
		return obj;
	}
	catch( Pcre::exception ex ) {
		LOG(ERROR) << ex.what();
		return SquirrelObject();
	}
}
SquirrelObject RegularExpression::splitWithLimitStartEndOffset(const std::string& piece, int limit, int start_offset, int end_offset)
{
	try {
		std::vector<std::string> res = Pcre::split(piece,limit,start_offset,end_offset);
		SquirrelObject obj = SquirrelVM::CreateArray(res.size());
		for( int i = 0; i  < res.size(); i++ ) {
			obj.SetValue(i, res[i].c_str());
		}
		return obj;
	}
	catch( Pcre::exception ex ) {
		LOG(ERROR) << ex.what();
		return SquirrelObject();
	}
}

const std::string RegularExpression::getMatch(int pos) const
{
	try {
		return Pcre::get_match(pos);
	}
	catch( Pcre::exception ex ) {
		LOG(ERROR) << ex.what();
		return std::string();
	}
}

bool RegularExpression::search(const std::string& stuff)
{
	try {
		return Pcre::search(stuff);
	} catch( Pcre::exception ex ) {
		LOG(ERROR) << ex.what();
		return false;
	}
}

bool RegularExpression::searchWithOffset(const std::string& stuff, int OffSet)
{
	try {
		return Pcre::search(stuff, OffSet);
	} catch( Pcre::exception ex ) {
		LOG(ERROR) << ex.what();
		return false;
	}
}

const std::string RegularExpression::replace(const std::string& piece, const std::string& with)
{
	try {
		return Pcre::replace(piece, with);
	} catch( Pcre::exception ex ) {
		LOG(ERROR) << ex.what();
		return std::string();
	}
}

int RegularExpression::getEntireMatchStart() const
{
	return Pcre::get_match_start();
}

int RegularExpression::getMatchStart(int pos) const
{
	try {
		return Pcre::get_match_start(pos);
	} catch( Pcre::exception ex ) {
		LOG(ERROR) << ex.what();
		return 0;
	}
}

int RegularExpression::getEntireMatchEnd() const
{
	return Pcre::get_match_end();
}

int RegularExpression::getMatchEnd(int pos) const
{
	try {
		return Pcre::get_match_end(pos);
	} catch( Pcre::exception ex ) {
		LOG(ERROR) << ex.what();
		return 0;
	}
}

bool RegularExpression::matched()
{
	return Pcre::matched();
}

int RegularExpression::matchesCount()
{
	return Pcre::matches();
}

SquirrelObject RegularExpression::getSubStrings()
{
	try {
		std::vector<std::string>* substrings =  Pcre::get_sub_strings();
		if ( !substrings ) {
			return SquirrelObject();
		}
		SquirrelObject res = SquirrelVM::CreateArray(substrings->size());
		for ( int i = 0; i < substrings->size(); i++ ) {
			res.SetValue(i, (*substrings)[i].c_str());
		}
		return res;
	} catch( Pcre::exception ex ) {
		LOG(ERROR) << ex.what();
		return SquirrelObject();
	}
}

bool RegularExpression::match(const std::string& stuff)
{
	return search(stuff);
}

SquirrelObject RegularExpression::findAll(const std::string& str)
{
	try {
		size_t pos = 0;
		SquirrelObject res = SquirrelVM::CreateArray(0);
		while (pos <= str.length()) 
		{
			if ( Pcre::search(str, pos)) 
			{ 
				pos = get_match_end()+1;
				int count = matchesCount();
				SquirrelObject mat = SquirrelVM::CreateArray(count);
				for ( int i = 0; i < count; i++ ) {
					mat.SetValue(i, get_match(i).c_str());
				}
				res.ArrayAppend(mat);
			}
			else {
				break;
			}
		}
		return res;
	} catch( Pcre::exception ex ) {
		LOG(ERROR) << ex.what();
		return SquirrelObject();
	}
}

ScriptAPI::RegularExpression* CreateRegExp(const std::string& expression, const std::string& flags)
{
	return new RegularExpression(expression,flags);
}

void RegisterRegularExpressionClass() {

	using namespace SqPlus;
	SQClassDef<RegularExpression>("RegularExpression")
		.func(&RegularExpression::search, "search")
		.func(&RegularExpression::findAll, "findAll")
		.func(&RegularExpression::match, "match")
		.func(&RegularExpression::searchWithOffset, "searchWithOffset")
		.func(&RegularExpression::getMatch, "getMatch")
		.func(&RegularExpression::replace, "replace")
		.func(&RegularExpression::matched, "matched")
		.func(&RegularExpression::matchesCount, "matchesCount")
		.func(&RegularExpression::getSubStrings, "getSubStrings")
		.func(&RegularExpression::split, "split")
		.func(&RegularExpression::splitWithLimitOffset, "splitWithLimitOffset")
		.func(&RegularExpression::splitWithLimitStartEndOffset, "splitWithLimitStartEndOffset")
		.func(&RegularExpression::getMatchStart, "getMatchStart")
		.func(&RegularExpression::getMatchEnd, "getMatchEnd")
		.func(&RegularExpression::getEntireMatchStart, "getEntireMatchStart")
		.func(&RegularExpression::getEntireMatchEnd, "getEntireMatchEnd");

	RegisterGlobal(CreateRegExp, "CRegExp");	
}

}