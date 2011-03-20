#ifndef _IU_CORE_UTILS_H
#define _IU_CORE_UTILS_H

#include <cstdio>
#include <string>


typedef std::string Utf8String;

namespace IuCoreUtils
{
	FILE * fopen_utf8(const char * filename, const char * mode);
	bool FileExists(const Utf8String fileName);
	Utf8String ExtractFileName(const Utf8String fileName);
	Utf8String ExtractFileExt(const Utf8String fileName);
	Utf8String ExtractFileNameNoExt(const Utf8String fileName);
	Utf8String toString(int value);
	Utf8String GetFileMimeType(const Utf8String);
	Utf8String StrReplace(Utf8String text, Utf8String s, Utf8String d);
	Utf8String ConvertToUtf8(const Utf8String &text, const Utf8String codePage);
	bool ReadUtf8TextFile(Utf8String utf8Filename, Utf8String& data);

	const std::wstring Utf8ToWstring(const Utf8String &str);
	const Utf8String WstringToUtf8(const std::wstring &str);
};
#endif
