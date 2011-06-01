#include <stdio.h>
#include "Core/3rdpart/utf8.h"
#include <sstream> // ostringstream
#include <iostream>
#include "StringUtils.h"

namespace IuCoreUtils
{

const std::wstring Utf8ToWstring(const std::string &str)
{
   using namespace utf8;
   std::wstring res;
   try
   {
      if(sizeof(wchar_t) == 2)
      {
         utf8to16(str.begin(), str.end(), back_inserter(res));
      }
      else if(sizeof(wchar_t) == 4)
      {
          utf8to32(str.begin(), str.end(), back_inserter(res));
      }
   }
   catch(...){}
   return res;
}

const Utf8String WstringToUtf8(const std::wstring &str)
{
   using namespace utf8;
   std::string res;
   try
   {
      if(sizeof(wchar_t) == 2)
      {
         utf16to8(str.begin(), str.end(), back_inserter(res));
      }
      else if(sizeof(wchar_t) == 4)
      {
          utf16to8(str.begin(), str.end(), back_inserter(res));
      }
   }
   catch(...){}
   return res;
}

Utf8String ConvertToUtf8(const Utf8String &text, const Utf8String codePage)
{
    // FIXME: stub
   return text;
}

Utf8String GetFileMimeType(const Utf8String name)
{
	std::string defaultType = "application/octet-stream";
	FILE* stream = popen( Utf8ToSystemLocale("file -b --mime-type '" + name + "'").c_str(), "r" );
	if(!stream) 
		return defaultType;
    std::ostringstream output;

    while( !feof( stream ) && !ferror( stream ))
    {
        char buf[128];
        int bytesRead = fread( buf, 1, 128, stream );
        output.write( buf, bytesRead );
    }
	pclose(stream);
   std::string result = IuStringUtils::Trim(output.str());
   return result;
}

}
