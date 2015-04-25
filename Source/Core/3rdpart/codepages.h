/*
http://rsdn.ru/forum/src/257031.1.aspx
codepagebyname.h 
*/
#ifndef INCLUDE_CODEPAGEBYNAME_H
#define INCLUDE_CODEPAGEBYNAME_H

#include <string.h>
int CodepageByName(const std::string& name);
std::string NameByCodepage( int code );
#endif