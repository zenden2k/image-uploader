#include "RegularExpression.h"
#include <Core/Logging.h>
using namespace pcrepp;

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
	std::vector<std::string> res = Pcre::split(piece);
	SquirrelObject obj = SquirrelVM::CreateArray(res.size());
	for( int i = 0; i  < res.size(); i++ ) {
		obj.SetValue(i, res[i].c_str());
	}
	return obj;
}

SquirrelObject RegularExpression::split_with_limit_offset(const std::string& piece, int limit, int start_offset)
{
	std::vector<std::string> res = Pcre::split(piece,limit,start_offset);
	SquirrelObject obj = SquirrelVM::CreateArray(res.size());
	for( int i = 0; i  < res.size(); i++ ) {
		obj.SetValue(i, res[i].c_str());
	}
	return obj;
}

SquirrelObject RegularExpression::split_with_limit(const std::string& piece, int limit)
{
	std::vector<std::string> res = Pcre::split(piece,limit);
	SquirrelObject obj = SquirrelVM::CreateArray(res.size());
	for( int i = 0; i  < res.size(); i++ ) {
		obj.SetValue(i, res[i].c_str());
	}
	return obj;
}
SquirrelObject RegularExpression::split_with_limit_start_end_offset(const std::string& piece, int limit, int start_offset, int end_offset)
{
	std::vector<std::string> res = Pcre::split(piece,limit,start_offset,end_offset);
	SquirrelObject obj = SquirrelVM::CreateArray(res.size());
	for( int i = 0; i  < res.size(); i++ ) {
		obj.SetValue(i, res[i].c_str());
	}
	return obj;
}

const std::string RegularExpression::get_match(int pos) const
{
	try {
		return Pcre::get_match(pos);
	}
	catch( std::exception ex ) {
		LOG(ERROR) << ex.what();
	}
}

bool RegularExpression::search(const std::string& stuff)
{
	return Pcre::search(stuff);
}

bool RegularExpression::search_with_offset(const std::string& stuff, int OffSet)
{
	return Pcre::search(stuff, OffSet);
}

const std::string RegularExpression::replace(const std::string& piece, const std::string& with)
{
	return Pcre::replace(piece, with);
}

int RegularExpression::get_entire_match_start() const
{
	return Pcre::get_match_start();
}

int RegularExpression::get_match_start(int pos) const
{
	return Pcre::get_match_start(pos);
}

int RegularExpression::get_entire_match_end() const
{
	return Pcre::get_match_end();
}

int RegularExpression::get_match_end(int pos) const
{
	return Pcre::get_match_end(pos);
}

bool RegularExpression::matched()
{
	return Pcre::matched();
}

int RegularExpression::matches()
{
	return Pcre::matches();
}

SquirrelObject RegularExpression::get_sub_strings()
{
	std::vector<std::string>* substrings =  Pcre::get_sub_strings();
	if ( !substrings ) {
		return SquirrelObject();
	}
	SquirrelObject res = SquirrelVM::CreateArray(substrings->size());
	for ( int i = 0; i < substrings->size(); i++ ) {
		res.SetValue(i, (*substrings)[i].c_str());
	}
	return res;
}



ScriptAPI::RegularExpression* CreateRegExp(const std::string& expression, const std::string& flags)
{
	return new RegularExpression(expression,flags);
}

}