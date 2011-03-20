#ifndef _IU_CORE_UTILS_H
#define _IU_CORE_UTILS_H

#include <cstdio>
#include <string>


typedef std::string Utf8String;
#ifdef _MSC_VER
	#define zint64 __int64
#else
	#define zint64  long long
#endif

namespace IuCoreUtils
{
	FILE * fopen_utf8(const char * filename, const char * mode);
	bool FileExists(const Utf8String fileName);
	bool DirectoryExists(const Utf8String path);
	Utf8String ExtractFilePath(const Utf8String fileName);
	Utf8String ExtractFileName(const Utf8String fileName);
	Utf8String ExtractFileExt(const Utf8String fileName);
	Utf8String ExtractFileNameNoExt(const Utf8String fileName);
	Utf8String toString(int value);
	Utf8String toString(unsigned int value);
	Utf8String toString(double value, int precision);
	Utf8String zint64ToString(zint64 value);
	zint64 stringTozint64(const Utf8String fileName);
	Utf8String GetFileMimeType(const Utf8String);
	Utf8String StrReplace(Utf8String text, Utf8String s, Utf8String d);
	Utf8String ConvertToUtf8(const Utf8String &text, const Utf8String codePage);
	bool ReadUtf8TextFile(Utf8String utf8Filename, Utf8String& data);
	zint64 getFileSize(Utf8String utf8Filename);
	const std::wstring Utf8ToWstring(const Utf8String &str);
	const Utf8String WstringToUtf8(const std::wstring &str);
	const std::string CalcMD5Hash(const void* data, size_t size);
	const std::string CalcMD5Hash(const std::string &data);
	const std::string timeStampToString(time_t t);
	Utf8String fileSizeToString(zint64 nBytes);
	bool createDirectory(const Utf8String path);
};
#endif
