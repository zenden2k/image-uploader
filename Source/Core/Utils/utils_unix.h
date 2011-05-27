#include "../3rdpart/utf8.h"

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

Utf8String GetFileMimeType(const Utf8String)
{
   // FIXME: stub
   return "application/unknown";
}

}
