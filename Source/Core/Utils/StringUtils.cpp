#include "StringUtils.h"
#include <cstdio>

namespace IuStringUtils
{
	std::string Trim(const std::string& str)
{
	std::string res;
	// Trim Both leading and trailing spaces
	size_t startpos = str.find_first_not_of(" \t\r\n"); // Find the first character position after excluding leading blank spaces
	size_t endpos = str.find_last_not_of(" \t\r\n"); // Find the first character position from reverse af

	// if all spaces or empty return an empty string
	if(( std::string::npos == startpos ) || ( std::string::npos == endpos))
	{
       res = "";
	}
   else
       res = str.substr( startpos, endpos-startpos+1 );
	return res;
}

void Split(const std::string& str, const std::string& delimiters, std::vector<std::string>& tokens, int maxCount)
{
    // Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
	int counter =0;
    while (std::string::npos != pos || std::string::npos != lastPos)
    {
		 counter++;
		 if(counter == maxCount){
			 tokens.push_back(str.substr(lastPos, str.length()));break;
		 }
		 else

        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

Utf8String Replace(const Utf8String& text, const Utf8String& s, const Utf8String& d)
{
	std::string result = text;
	for(unsigned index=0; index=result.find(s, index), index!=std::string::npos;)
	{
		result.replace(index, s.length(), d);
		index += d.length();
	}
	return result;
}
}
