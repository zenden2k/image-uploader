#include "CoreUtils.h"
#include <cstdio>
//#include "../../myutils.h"
#include <io.h>

#include "Utils_Win.h"
namespace IuCoreUtils
{

FILE * fopen_utf8(const char * filename, const char * mode)
{
	#ifdef _WIN32
	return _wfopen(Utf8ToWstring(filename).c_str(), Utf8ToWstring(mode).c_str());
	#endif
}

bool FileExists(Utf8String fileName)
{
	#ifdef WIN32
		if(GetFileAttributes(Utf8ToWstring(fileName).c_str())== (unsigned long)-1) return false;
	#else
		//TODO
	#endif
	return true;
}

Utf8String ExtractFileName(const Utf8String fileName)
{
		Utf8String temp = fileName;
		int Qpos = temp.find_last_of('?');
		if(Qpos>=0) temp = temp.substr(0, Qpos-1);
		int i,len = temp.length();
		for(i=len-1; i>=0; i--)
		{
			if(temp[i] == '\\' || temp[i]=='/')
				break;
		}
		return temp.substr(i+1);
}

Utf8String ExtractFileExt(const Utf8String fileName)
{
	int nLen = fileName.length();

	Utf8String result;
	for( int i=nLen-1; i>=0; i-- )
	{
		if(fileName[i] == '.')
		{
			result = fileName.substr(i + 1);
			break;
		}
		else if(fileName[i] == '\\' || fileName[i] == '/') break;
	}
	return result;
}

Utf8String ExtractFileNameNoExt(const Utf8String fileName)
{
	Utf8String result = ExtractFileName(fileName);
	int Qpos = result.find_last_of('.');
	if(Qpos>=0) result = result.substr(0, Qpos-1);
	return result;
}

Utf8String toString(int value)
{
	char buffer[256];
	sprintf(buffer, "%d", value);
	return buffer;
}

Utf8String StrReplace(Utf8String text, Utf8String s, Utf8String d)
{
	for(unsigned index=0; index=text.find(s, index), index!=std::string::npos;)
	{
		text.replace(index, s.length(), d);
		index += d.length();
	}
	return text;
}


bool ReadUtf8TextFile(Utf8String utf8Filename, Utf8String& data)
{
	FILE *stream = fopen_utf8(utf8Filename.c_str(), "rb");
	if(!stream) return false;
	long size = _filelength(fileno(stream));	
	unsigned char buf[3];
	fread(buf, 1, 3, stream);	


	if(buf[0] == 0xEF || buf[1] == 0xBB || buf[2] == 0xBF)
	{
		size -= 3;
		
	}
	else if(buf[0] == 0xFF || buf[1] == 0xFE )
	{
		size -= 2;
		fseek( stream, 2L,  SEEK_SET );
		std::wstring res;
		int charCount = size/2;
		res.resize(charCount);
		size_t charsRead = fread(&res[0], 2, charCount, stream);	
		res[charsRead]=0;
		fclose(stream);
		data = WstringToUtf8(res);
		return true;
	}

	else 
	{
		
		fseek( stream, 0L,  SEEK_SET );
	}
	data.resize(size+1);
	size_t bytesRead = fread(&data[0], 1, size, stream);	
	data[bytesRead]=0;
	fclose(stream);
	return true;
}



}
