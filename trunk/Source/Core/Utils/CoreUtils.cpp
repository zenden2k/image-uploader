#include "CoreUtils.h"
#include <cstdio>
//#include "../../myutils.h"
#include <io.h>
#include "Utils_Win.h"
#include <openssl/md5.h>
#include <time.h> 
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

Utf8String ExtractFilePath(const Utf8String fileName)
{
	int i, len = fileName.length();
	for(i=len; i>=0; i--)
	{
		if(fileName[i] == '\\' || fileName[i]=='/')
		{
			return fileName.substr(0, i);
		}
			
	}
	return "";
}

Utf8String ExtractFileNameNoExt(const Utf8String fileName)
{
	Utf8String result = ExtractFileName(fileName);
	int Qpos = result.find_last_of('.');
	if(Qpos>=0) result = result.substr(0, Qpos);
	return result;
}

Utf8String toString(int value)
{
	char buffer[256];
	sprintf(buffer, "%d", value);
	return buffer;
}

Utf8String toString(unsigned int value)
{
	char buffer[256];
	sprintf(buffer, "%u", value);
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
	long size = _filelength(_fileno(stream));	
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

const std::string CalcMD5Hash(const void* data, size_t size)
{
	std::string result;
	MD5_CTX context;

	MD5_Init(&context);
	MD5_Update(&context, data, size);

	unsigned char buff[16] = "";    

	MD5_Final(buff, &context);

	for(int i=0;i<16; i++)
	{
		char temp[5];
		sprintf(temp, "%02x",buff[i]);
		result += temp;
	}
	return result;
}



// FIXME: I don't know if it works with binary data
const std::string CalcMD5Hash(const std::string& data)
{
	std::string result;
	MD5_CTX context;

	MD5_Init(&context);
	MD5_Update(&context, (unsigned char*)data.c_str(), data.length());

	unsigned char buff[16] = "";    

	MD5_Final(buff, &context);

	for(int i=0;i<16; i++)
	{
		char temp[5];
		sprintf(temp, "%02x",buff[i]);
		result += temp;
	}
	return result;
}

const std::string timeStampToString(time_t t)
{
	// TODO: add system locale support
	tm * timeinfo = localtime ( &t );
	char buf[50];
	sprintf(buf, "%02d.%02d.%04d %02d:%02d:%02d", (int)timeinfo->tm_mday,(int) timeinfo->tm_mon+1, (int)1900+timeinfo->tm_year, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); 
	return buf;
}

Utf8String zint64ToString(zint64 value)
{
	char buf[200];
	_i64toa(value, buf, 10);
	return buf;
}

zint64 stringTozint64(const Utf8String fileName)
{
	return _atoi64(fileName.c_str());
}

zint64 getFileSize(Utf8String utf8Filename)
{
	_stat64 stats;
	 _wstati64(Utf8ToWstring(utf8Filename).c_str(), &stats); 
	 return stats.st_size;
}

// Преобразование размера файла в строку
Utf8String fileSizeToString(zint64 nBytes)
{
	double number=0;
	int id=0;
	Utf8String postfix;
	int precision = 0;
	if(nBytes < 0)
	{
		return "n/a";
	}

	if(nBytes < 1024)
	{
		number = double(nBytes);
		postfix = "bytes";
	}
	else if( nBytes < 1048576)
	{
		number = (double)nBytes / 1024.0;
		postfix = "kB";
	}
	else if(nBytes<(__int64(1073741824))) /*< 1 GB*/
	{
		postfix= "mB";
		number= (double)nBytes / 1048576.0;
	}
	else if(nBytes>=1073741824)
	{
		postfix= "gB";
		precision = 1;
		number = (double)nBytes / 1073741824.0;
	}
	return toString(number, precision) + " " + postfix;
}

Utf8String toString(double value, int precision)
{
	char buffer[100];
	sprintf(buffer, ("%0." + toString(precision) + "f").c_str(), value);
	return buffer;
}

}
