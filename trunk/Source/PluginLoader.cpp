/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdafx.h>
#include "PluginLoader.h"
#include <stdio.h>
#include <stdarg.h>
#undef UNICODE
#undef _UNICODE
#include <sqplus.h>
#include <sqstdsystem.h>
#include <windows.h>
#include "Common/networkmanager.h"
#include <io.h>

using namespace SqPlus;

#ifdef _DEBUG
	#pragma comment(lib,"squirrelD.lib")
	#pragma comment(lib,"sqstdlibD.lib")
	#pragma comment(lib,"sqplusD.lib")
#else
	#pragma comment(lib,"squirrel.lib")
	#pragma comment(lib, "sqstdlib.lib")
	#pragma comment(lib,"sqplus.lib")
#endif

#pragma comment(lib, "libeay32.lib")

#include "Plugins/CP_RSA.h"
#include "Plugins/base64.h"
#include "common.h"
#include <openssl/md5.h>
 
const std::string IU_md5(const std::string& data)
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


int pluginRandom()
{
	return rand();
}

bool ReadUtf8TextFile(CString filename, std::string& data)
{
	FILE *stream = _wfopen(filename,_T("rb"));
	if(!stream) return false;
	long size = _filelength(fileno(stream));	
	unsigned char buf[3];
	fread(buf, 1, 3, stream);	

	if(buf[0]!=0xEF || buf[1]!=0xBB || buf[2]!=0xBF)
	{
		fseek( stream, 0L,  SEEK_SET );
	}
	else size-=3;
	data.resize(size+1);
	
	size_t bytesRead = fread(&data[0], 1, size, stream);	
	data[size]=0;
	fclose(stream);
}

CString IU_md5_file(const CString& filename)
{
	CString result;
	MD5_CTX context;

	MD5_Init(&context);
	FILE *f = _wfopen(filename,_T("rb"));
	
	if(f)
	{
		unsigned char buf[4096];
		while(!feof(f))
		{	
			size_t bytesRead = fread(buf, 1, sizeof(buf), f);


			MD5_Update(&context, (unsigned char*)buf, bytesRead);
		}
		unsigned char buff[16] = "";    
		  
		MD5_Final(buff, &context);
		
		fclose(f);

		for(int i=0; i<16; i++)
		{
			TCHAR temp[5];
			swprintf(temp, _T("%02x"),buff[i]);
			result += temp;
		}
	}
	return result;
}

const std::string YandexRsaEncrypter(const std::string& key, const std::string& data)
{
	CCryptoProviderRSA encrypter;
	encrypter.ImportPublicKey(key.c_str());	
	char crypted_data[MAX_CRYPT_BITS / sizeof(char)] = "\0";
	size_t crypted_len = 0;
	encrypter.Encrypt(data.c_str(), data.length(), crypted_data, crypted_len);

	return base64_encode((unsigned char *)crypted_data, crypted_len);
}

const std::string scriptGetFileMimeType(const std::string& filename)
{
	return WstringToUtf8((LPCTSTR)IU_GetFileMimeType(Utf8ToWstring(filename).c_str()));
}

void CFolderList::AddFolder(const std::string& title, const std::string& summary, const std::string& id, const std::string& parentid, int accessType)
{
	CFolderItem ai;
	ai.title = Utf8ToWstring(title);
	ai.summary = Utf8ToWstring(summary);
	ai.id = Utf8ToWstring(id);
	ai.parentid =  Utf8ToWstring(parentid);
	ai.accessType = accessType;
	m_folderItems.push_back(ai);
}

void CFolderList::AddFolderItem(const CFolderItem& item)
{
	m_folderItems.push_back(item);
}

DECLARE_INSTANCE_TYPE(CFolderList);
DECLARE_INSTANCE_TYPE(CFolderItem);
DECLARE_INSTANCE_TYPE(NetworkManager);
DECLARE_INSTANCE_TYPE(ServerSettingsStruct);
DECLARE_INSTANCE_TYPE(CIUUploadParams);

CString squirrelOutput;

static void printFunc(HSQUIRRELVM v,const SQChar * s,...) 
{
	va_list vl;
	va_start(vl,s);
	int len = _vscprintf( s,vl ) +1;
	char * buffer = new char [len];
	vsprintf( buffer,s,vl);
	va_end(vl);
	std::wstring text =  Utf8ToWstring(buffer);
	squirrelOutput+= text.c_str();
	delete[] buffer;	
} 

void FlushSquirrelOutput()
{
	if(!squirrelOutput.IsEmpty())
	{
		WriteLog(logWarning, _T("Squirrel"), squirrelOutput);
		squirrelOutput.Empty();
	}

}
LPCSTR ExtractFileNameA(LPCSTR FileName)
{  
	int i,len = lstrlenA(FileName);
	for(i=len; i>=0; i--)
	{
		if(FileName[i] == _T('\\'))
			break;
	}
	return FileName+i+1;
}

LPCSTR GetFileExtA(LPCSTR szFileName)
{
	if(!szFileName) return 0;
	int nLen = lstrlenA(szFileName);
	
	LPCSTR szReturn = szFileName+nLen;
	for( int i=nLen-1; i>=0; i-- )
	{
		if(szFileName[i] == '.')
		{
			szReturn = szFileName + i + 1;
			break;
		}
		else if(szFileName[i] == '\\') break;
	}
	return szReturn;
}

const std::string plugExtractFileName(const std::string& path)
{
	std::string res = ExtractFileNameA(path.c_str());
	return res;
}

const std::string plugGetFileExtension(const std::string& path)
{
	std::string res = GetFileExtA(path.c_str());
	return res;
}

CUploadScript::CUploadScript(LPCTSTR pluginName)
{
	
	m_sName = pluginName;
	m_CreationTime = GetTickCount(); 
	sq_setprintfunc(SquirrelVM::GetVMPtr(),printFunc);
}

CUploadScript::~CUploadScript()
{
	//SquirrelVM::Shutdown();
}

void CUploadScript::setServerParams(ServerSettingsStruct& params)
{
	m_pServerParams = &params;
}
bool CUploadScript::load(LPCTSTR fileName, ServerSettingsStruct& params)
{  
	if(!FileExists(fileName))
		return false;
	
	setServerParams(params);
	
	m_Object = SquirrelVM::CreateTable();

	BindVariable(m_Object, m_pServerParams, "ServerParams");
	try
	{
		std::string scriptText;
		ReadUtf8TextFile(fileName, scriptText);
		m_SquirrelScript = SquirrelVM::CompileBuffer(scriptText.c_str(), WCstringToUtf8( myExtractFileName(fileName)).c_str());
		SquirrelVM::RunScript(m_SquirrelScript, &m_Object);
	}
	catch (SquirrelError & e) {
		MessageBoxA(0, e.desc,"bool CUploadScript::load(LPCTSTR fileName, ServerSettingsStruct& params),0);",0);
		return false;
  } 
	FlushSquirrelOutput();
	return true;
}

int CUploadScript::uploadFile(LPCWSTR FileName, CIUUploadParams &params)
{
	int ival = 0;
	try {
		SquirrelFunction<int> func(m_Object, _SC("UploadFile"));
		std::string fname = WstringToUtf8(FileName);
		ival = func(fname.c_str(), &params); // Argument coun*/
	}
	catch (SquirrelError & e) {
    printf("Error: %s, %s\n",e.desc,"Squirrel::helloSqPlus");
	 MessageBoxA(0,e.desc,0,0);
  } 
	FlushSquirrelOutput();
	return ival;
}
int CUploadScript::getAccessTypeList(std::vector<std::wstring> &list)
{
	try {
		SquirrelFunction<SquirrelObject> func(m_Object, _SC("GetFolderAccessTypeList"));
		if(func.func.IsNull()) return -1;
		SquirrelObject arr = func();

		list.clear();
		int count =  arr.Len();
		for(int i=0; i<count; i++){
			std::string title;
			title = arr.GetString(i);
			list.push_back(Utf8ToWstring(title));
		}
	}
	catch (SquirrelError & e) {
		printf("Error: %s, %s\n",e.desc,"Squirrel::helloSqPlus");
  } 
	FlushSquirrelOutput();
	return 1;

}

int CUploadScript::getServerParamList(std::map<std::wstring,std::wstring> &list)
{
	SquirrelObject arr;

	try {
		SquirrelFunction<SquirrelObject> func(m_Object, _SC("GetServerParamList"));
		if(func.func.IsNull()) return -1;
		arr = func();

		list.clear();
		SquirrelObject key, value;
		arr.BeginIteration();
		while(arr.Next(key, value))
		{
			std::string title = value.ToString();
			list[Utf8ToWstring( key.ToString())]=Utf8ToWstring( title);
		}
		arr.EndIteration();

	}
	catch (SquirrelError & e) {
		printf("Error: %s, %s\n",e.desc,"Squirrel::helloSqPlus");
  } 
	FlushSquirrelOutput();
	return 1;

}

int   CUploadScript::modifyFolder(CFolderItem &folder)
{
	try {
		SquirrelFunction<int> func(m_Object, _SC("ModifyFolder"));
		if(func.func.IsNull()) return -1;
		int ival = func(&folder);
	}
	catch (SquirrelError & e) {
		printf("Error: %s, %s\n",e.desc,"Squirrel::helloSqPlus");
  } 
	FlushSquirrelOutput();
	return 1;
}

int CUploadScript::getFolderList(CFolderList &FolderList)
{
	int ival = 0;
	try
	{
		SquirrelFunction<int> func(m_Object, _SC("GetFolderList"));
		if(func.func.IsNull()) return -1;
		ival = func(&FolderList); 
	}
	catch (SquirrelError & e) 
	{
		MessageBoxA(0, e.desc,"int CUploadScript::getFolderList(CFolderList &FolderList)" ,0);
		//printf("Error: %s, %s\n",e.desc,"Squirrel::helloSqPlus");
  } 
	FlushSquirrelOutput();
	return ival;
}



bool CUploadScript::isLoaded()
{
	return m_bIsPluginLoaded;
}

CString CUploadScript::name()
{
	return m_sName;
}

int  CUploadScript::createFolder(CFolderItem &parent, CFolderItem &folder)
{
	int ival = 0;
	try
	{
		SquirrelFunction<int> func(m_Object, _SC("CreateFolder"));
		if(func.func.IsNull()) return -1;
		ival = func(&parent,&folder); // Argument coun*/
	}
	catch (SquirrelError & e) 
	{
		printf("Error: %s, %s\n",e.desc,"Squirrel::helloSqPlus");
	
	} 
	FlushSquirrelOutput();
	return ival;	
} 
		
DWORD CUploadScript::getCreationTime()
{
	return m_CreationTime;
}

CPluginManager:: CPluginManager()
{
	SquirrelVM::Init();
	SquirrelVM::PushRootTable();
			
	sqstd_register_systemlib(SquirrelVM::GetVMPtr());
	
	SQClassDef<NetworkManager>("NetworkManager").
		func(&NetworkManager::doGet,"doGet").
		func(&NetworkManager::responseBody,"responseBody").
		func(&NetworkManager::responseCode,"responseCode").
		func(&NetworkManager::setUrl,"setUrl").
		func(&NetworkManager::doPost,"doPost").
		func(&NetworkManager::addQueryHeader,"addQueryHeader").
		func(&NetworkManager::addQueryParam,"addQueryParam").
		func(&NetworkManager::addQueryParamFile,"addQueryParamFile").
		func(&NetworkManager::responseHeaderCount,"responseHeaderCount").
		func(&NetworkManager::urlEncode,"urlEncode").
		func(&NetworkManager::errorString,"errorString").
		func(&NetworkManager::doUpload,"doUpload").
		func(&NetworkManager::setMethod,"setMethod").
		func(&NetworkManager::doUploadMultipartData,"doUploadMultipartData");
	
	SQClassDef<CFolderList>("CFolderList").
		func(&CFolderList::AddFolder,"AddFolder").
		func(&CFolderList::AddFolderItem,"AddFolderItem");
	
	SQClassDef<ServerSettingsStruct>("ServerSettingsStruct").
		func(&ServerSettingsStruct::setParam,"setParam").
		func(&ServerSettingsStruct::getParam,"getParam");

	SQClassDef<CIUUploadParams>("CIUUploadParams").
		func(&CIUUploadParams::getFolderID,"getFolderID").
		func(&CIUUploadParams::setDirectUrl,"setDirectUrl").
		func(&CIUUploadParams::setThumbUrl,"setThumbUrl").
		func(&CIUUploadParams::getServerFileName,"getServerFileName").
		func(&CIUUploadParams::setViewUrl,"setViewUrl");	

	SQClassDef<CFolderItem>("CFolderItem").
		func(&CFolderItem::getId,"getId").
		func(&CFolderItem::getParentId,"getParentId").
		func(&CFolderItem::getSummary,"getSummary").
		func(&CFolderItem::getTitle,"getTitle").
		func(&CFolderItem::setId,"setId").
		func(&CFolderItem::setParentId,"setParentId").
		func(&CFolderItem::setSummary,"setSummary").
		func(&CFolderItem::setTitle,"setTitle").
		func(&CFolderItem::getAccessType,"getAccessType").
		func(&CFolderItem::setAccessType,"setAccessType").
		func(&CFolderItem::setItemCount,"setItemCount").
		func(&CFolderItem::setViewUrl,"setViewUrl").			
		func(&CFolderItem::getItemCount,"getItemCount");

	RegisterGlobal(pluginRandom, "random");
		RegisterGlobal(IU_md5, "md5");
		RegisterGlobal(AnsiToUtf8, "AnsiToUtf8");
		RegisterGlobal(YandexRsaEncrypter,"YandexRsaEncrypter");
		RegisterGlobal(Utf8ToAnsi, "Utf8ToAnsi");
	   RegisterGlobal(plugExtractFileName, "ExtractFileName");
		RegisterGlobal(plugGetFileExtension, "GetFileExtension");
		RegisterGlobal(scriptGetFileMimeType, "GetFileMimeType");
		srand(time(0));
}
CUploadScript* CPluginManager::getPlugin(LPCTSTR name, ServerSettingsStruct& params, bool UseExisting)
{
	DWORD curTime = GetTickCount();
	if(m_plugin && (GetTickCount() - m_plugin->getCreationTime()<1000*60*5))
		UseExisting = true;

	if(m_plugin && UseExisting && m_plugin->name() == CString(name))
	{
		return m_plugin;
	}

	if(m_plugin)
	{	
		delete m_plugin; 
		m_plugin = 0;
	}
		
	CUploadScript* newPlugin = new CUploadScript(name);
	if(newPlugin->load(IU_GetDataFolder()+"\\Scripts\\"+name+_T(".nut"), params))
	{
		m_plugin =newPlugin; 

		return newPlugin;
	}
	else 
		delete newPlugin;

	WriteLog(logError, _T("CUploadScript"),CString(_T("Unable to load plugin ")) + name);
	return NULL;
}

void CUploadScript::bindNetworkManager(NetworkManager * nm)
{
	BindVariable(m_Object, nm, "nm");
}

bool CUploadScript::supportsSettings()
{
	SquirrelObject arr;

	try {
		SquirrelFunction<SquirrelObject> func(m_Object, _SC("GetServerParamList"));
		if(func.func.IsNull()) return false;
		

	}
	catch (SquirrelError & e) {
		return false;
  } 
	//FlushSquirrelOutput();
	return true;

}
CPluginManager::~CPluginManager()
{
	delete m_plugin;
}

void CPluginManager::UnloadPlugins()
{
	if(m_plugin)
	delete m_plugin;
	m_plugin = NULL;
}