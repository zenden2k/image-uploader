/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ScriptUploadEngine.h"

#undef UNICODE
#undef _UNICODE
#include <sqrat.h>
/*#include <sqplus.h>*/
#include <sqstdsystem.h>
#include <sqstdstring.h>

#include "../../Plugins/CP_RSA.h"
#include "../../Plugins/base64.h"
//#include "../../common.h"
#include <openssl/md5.h>

#ifdef _MSC_VER
#ifdef _DEBUG
	#pragma comment(lib,"squirrelD.lib")
	#pragma comment(lib,"sqstdlibD.lib")
//	#pragma comment(lib,"sqplusD.lib")
#else
#pragma comment(lib,"squirrel.lib")
#pragma comment(lib, "sqstdlib.lib")
//#pragma comment(lib,"sqplus.lib")
#endif
#pragma comment(lib, "libeay32.lib")
#endif 

//using namespace SqPlus;

// Squirrel types should be defined in the same module where they are used
// otherwise we will catch SqPlus exception while executing Squirrel functions

/*DECLARE_INSTANCE_TYPE(ServerSettingsStruct);
DECLARE_INSTANCE_TYPE(NetworkManager);
DECLARE_INSTANCE_TYPE(CFolderList);
DECLARE_INSTANCE_TYPE(CFolderItem);
DECLARE_INSTANCE_TYPE(CIUUploadParams);*/

void CFolderList::AddFolder(const std::string& title, const std::string& summary, const std::string& id, const std::string& parentid, int accessType)
{
		  CFolderItem ai;
		  ai.title = (title);
		  ai.summary = (summary);
		  ai.id = (id);
		  ai.parentid =  (parentid);
		  ai.accessType = accessType;
		  m_folderItems.push_back(ai);
}


void CFolderList::AddFolderItem(const CFolderItem& item)
{
		  m_folderItems.push_back(item);
}

// Kind of hack
// older versions of Squirrel Standart Library have broken srand() function 
int pluginRandom()
{
	return rand();
}
using namespace Sqrat;
// FIXME: I don't know if it works with binary data
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

static SQInteger errorhandler(HSQUIRRELVM v) {
			const SQChar *sErr = 0;
			if(sq_gettop(v)>=1) {
				if(SQ_SUCCEEDED(sq_getstring(v,2,&sErr)))	{
					throw(Exception(sErr));
					//MessageBoxA(0,sErr,0,0);
					//ADD_FAILURE() << _SC("A Script Error Occured: ") << sErr;
				}
				else{
					//MessageBoxA(0,sErr,0,0);
					//ADD_FAILURE() << _SC("An Unknown Script Error Occured.") << sErr;
				}
			}
			return 0;
		}


std::string squirrelOutput;
const Utf8String IuNewFolderMark = "_iu_create_folder_";
static void printFunc(HSQUIRRELVM v,const SQChar * s,...) 
{
	va_list vl;
	va_start(vl,s);
	int len = 1024;//_vcsprintf( s,vl ) + 1;
	char * buffer = new char [len+1];
	vsnprintf( buffer,len, s,vl);
	va_end(vl);
	//std::wstring text =  Utf8ToWstring(buffer);
	squirrelOutput += buffer;
	delete[] buffer;	
} 

void CScriptUploadEngine::FlushSquirrelOutput()
{
	if(!squirrelOutput.empty())
	{
		Log(mtWarning, "Squirrel\r\n"+ squirrelOutput);
		//MessageBoxA(0,squirrelOutput.c_str(), 0 ,0);
		//qDebug()<<
		squirrelOutput.clear();
	}
}


bool CScriptUploadEngine::doUpload(Utf8String FileName, Utf8String DisplayName, CIUUploadParams &params)
{
	CFolderItem parent, newFolder = m_ServersSettings.newFolder;
	std::string folderID = m_ServersSettings.params["FolderID"];

	//MessageBoxA(0, FileName.c_str(), 0 ,0);
	if(folderID == IuNewFolderMark)
	{
		SetStatus(stCreatingFolder, newFolder.title);
		if( createFolder(parent,newFolder))
		{
			folderID = newFolder.id;
			m_ServersSettings.params["FolderID"] = folderID; 
			m_ServersSettings.params["FolderUrl"] = newFolder.viewUrl; 
		}
		else folderID.clear();
	}

	params.folderId = folderID;

	int ival = 0;
	try {
		Function func(RootTable(vm), "UploadFile");
		//SquirrelFunction<int> func(m_Object, _SC("UploadFile"));
		std::string fname = FileName;

		ival = func.Evaluate<int>(fname.c_str(), &params); // Argument coun*/
	}
	catch (Exception & e) {
		Log(mtError, "CScriptUploadEngine::uploadFile\r\n" + Utf8String(e.Message()));
	} 
	FlushSquirrelOutput();
	return ival;
	return 1;
}

const std::string plugExtractFileName(const std::string& path)
{
	std::string res = IuCoreUtils::ExtractFileName(path);
	return res;
}

const std::string plugGetFileExtension(const std::string& path)
{
	std::string res = IuCoreUtils::ExtractFileExt(path);
	return res;
}

bool CScriptUploadEngine::needStop()
{
	bool m_bShouldStop = false;
	if(onNeedStop)
		m_bShouldStop = onNeedStop(); // delegate call
	return m_bShouldStop;
}

CScriptUploadEngine::CScriptUploadEngine(Utf8String pluginName):CAbstractUploadEngine()
{	
	vm = sq_open(1024);
	//MessageBoxA(0,"Opening ",0,0);
	//Sqrat::DefaultVM::Set(vm);
	//sqstd_seterrorhandlers(vm); 
	m_sName = pluginName;
	m_CreationTime = GetTickCount(); 	
}

CScriptUploadEngine::~CScriptUploadEngine()
{
	/*sq_pop(vm,1); //pops the root table
	*/
	sq_close(vm); 
	vm = 0;
	
	//sq_close(vm);
	//SquirrelVM::Shutdown();
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
	return IuCoreUtils::GetFileMimeType(filename); 
}


const std::string AnsiToUtf8(const std::string &str, const std::string& codepage)
{
	return IuCoreUtils::ConvertToUtf8(str,codepage);
}

const std::string Utf8ToAnsi(const std::string &str, const std::string& codepage)
{
	// FIXME
	return IuCoreUtils::ConvertToUtf8(str,codepage);
}

void BindSquirrel(HSQUIRRELVM vm) 
{
	using namespace Sqrat;

	Class<CFolderList> CFolderList_Class(vm);

	CFolderList_Class
		.Func("AddFolder", &CFolderList::AddFolder)
		.Func("AddFolderItem", &CFolderList::AddFolderItem);

		RootTable(vm).Bind("CFolderList", CFolderList_Class);

	Class<NetworkManager> NetworkManager_Class(vm);

	NetworkManager_Class.
		Func("doGet", &NetworkManager::doGet).
		Func("responseBody",&NetworkManager::responseBody).
		Func("responseCode", &NetworkManager::responseCode).
		Func("setUrl",&NetworkManager::setUrl).
		Func("doPost",&NetworkManager::doPost).
		Func("addQueryHeader",&NetworkManager::addQueryHeader).
		Func("addQueryParam",&NetworkManager::addQueryParam).
		Func("addQueryParamFile",&NetworkManager::addQueryParamFile).
		Func("responseHeaderCount",&NetworkManager::responseHeaderCount).
		Func("urlEncode",&NetworkManager::urlEncode).
		Func("errorString",&NetworkManager::errorString).
		Func("doUpload",&NetworkManager::doUpload).
		Func("setMethod",&NetworkManager::setMethod).
		Func("doUploadMultipartData",&NetworkManager::doUploadMultipartData);

	RootTable(vm).Bind("NetworkManager", NetworkManager_Class);

		/*SQClassDef<NetworkManager>("NetworkManager").
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
*/

		
Class<CIUUploadParams> CIUUploadParams_Class(vm);
CIUUploadParams_Class.Func("getFolderID", &CIUUploadParams::getFolderID).
Func("setDirectUrl",&CIUUploadParams::setDirectUrl).
Func("setThumbUrl",&CIUUploadParams::setThumbUrl).
Func("getServerFileName",&CIUUploadParams::getServerFileName).
Func("setViewUrl",&CIUUploadParams::setViewUrl);

		RootTable(vm).Bind("CIUUploadParams", CIUUploadParams_Class);


		/*SQClassDef<CIUUploadParams>("CIUUploadParams").
				func(&CIUUploadParams::getFolderID,"getFolderID").
				func(&CIUUploadParams::setDirectUrl,"setDirectUrl").
				func(&CIUUploadParams::setThumbUrl,"setThumbUrl").
				func(&CIUUploadParams::getServerFileName,"getServerFileName").
				func(&CIUUploadParams::setViewUrl,"setViewUrl");*/

		RootTable(vm).Bind("CFolderItem", Class<CFolderItem>(vm).
								 Func("getId", &CFolderItem::getId).
								 Func("getParentId",&CFolderItem::getParentId).
								 Func("getSummary",&CFolderItem::getSummary).
								 Func("getTitle",&CFolderItem::getTitle).
								 Func("setId",&CFolderItem::setId).
								 Func("setParentId",&CFolderItem::setParentId).
								 Func("setSummary",&CFolderItem::setSummary).
								 Func("setTitle",&CFolderItem::setTitle).
								 Func("getAccessType",&CFolderItem::getAccessType).
								 Func("setAccessType",&CFolderItem::setAccessType).
								 Func("setItemCount",&CFolderItem::setItemCount).
								 Func("setViewUrl",&CFolderItem::setViewUrl).
								 Func("getItemCount",&CFolderItem::getItemCount)
		);

		RootTable(vm).Bind("ServerSettingsStruct", Class<ServerSettingsStruct>(vm)
			.Func("setParam", &ServerSettingsStruct::setParam)
			.Func("getParam", &ServerSettingsStruct::getParam)
			);
			/*SQClassDef<CFolderItem>("CFolderItem").
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
*/
		RootTable(vm).Func("random",pluginRandom).
		Func("md5",IU_md5).
		Func("AnsiToUtf8",AnsiToUtf8).
		Func("YandexRsaEncrypter",YandexRsaEncrypter).
		Func("Utf8ToAnsi",Utf8ToAnsi).
			Func("ExtractFileName",plugExtractFileName).
			Func("GetFileExtension",plugGetFileExtension).
			Func("GetFileMimeType",scriptGetFileMimeType);

			/*RegisterGlobal(plugExtractFileName, "ExtractFileName");
			RegisterGlobal(plugGetFileExtension, "GetFileExtension");
			RegisterGlobal(scriptGetFileMimeType, "GetFileMimeType");*/

	}

bool CScriptUploadEngine::load(Utf8String fileName, ServerSettingsStruct& params)
{  
	if(!IuCoreUtils::FileExists(fileName))
		return false;

	setServerSettings(params);
	try
	{

//	m_Object = SquirrelVM::CreateTable();

	//RootTable(vm).Bind("ServerParams",Sqrat::Var<ServerSettingsStruct>&m_ServersSettings>());
	//BindVariable(m_Object, &m_ServersSettings, "ServerParams");

		std::string scriptText;
		IuCoreUtils::ReadUtf8TextFile(fileName, scriptText);

		//m_SquirrelScript.
	

		//Script()

		
sq_pushroottable(vm);
		if(!vm) return false;
		BindSquirrel(vm);
			sq_setprintfunc(/*SquirrelVM::GetVMPtr()*/vm,printFunc,printFunc);
			 sqstd_register_systemlib(vm);
			 if(SQ_SUCCEEDED(sqstd_register_stringlib(vm)))
			 {
				 //Log(mtError, "CScriptUploadEngine::Load\r\n" + std::string("OK") );
			 }

				RootTable(vm).SetValue("ServerParams", &m_ServersSettings);
				/*sq_newclosure(vm, errorhandler,0);
						sq_seterrorhandler(vm);*/
		Sqrat::Script m_SquirrelScript(vm);
							m_SquirrelScript.CompileString(scriptText.c_str());


		m_SquirrelScript.Run();

		//m_SquirrelScript = SquirrelVM::CompileBuffer(scriptText.c_str(), IuCoreUtils::ExtractFileName(fileName).c_str());
		//SquirrelVM::RunScript(m_SquirrelScript, &m_Object);
	}
	catch (Exception & e) {
		//MessageBoxA(0, e.Message().c_str(),0,0);
		Log(mtError, "CScriptUploadEngine::Load\r\n" + std::string("Unable to load plugin: ") + e.Message());
		return false;
	} 

	try
	{

		/*
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
	RegisterGlobal(scriptGetFileMimeType, "GetFileMimeType");*/
	srand(time(0));
}
	catch (Exception & e) {
		//Log(mtError, "CScriptUploadEngine::Load\r\n" + std::string("Unable to load plugin: ") + e.desc);
		return false;
	}

	FlushSquirrelOutput();
	return true;
}

int CScriptUploadEngine::getAccessTypeList(std::vector<Utf8String> &list)
{
	try {
		Function func(RootTable(vm), "GetFolderAccessTypeList");

		//SquirrelFunction<SquirrelObject> func(m_Object, _SC("GetFolderAccessTypeList"));
		if(func.IsNull()) return -1;

//		func.Evaluate<Table>();
		//SquirrelObject arr = func();

		/*list.clear();
		int count =  arr.Len();
		for(int i=0; i<count; i++){
			std::string title;
			title = arr.GetString(i);
			list.push_back(title);
		}*/
	}
	catch (Exception & e) {
		Log(mtError, "CScriptUploadEngine::getAccessTypeList\r\n"+std::string(e.Message()));
	} 
	FlushSquirrelOutput();
	return 1;

}

int CScriptUploadEngine::getServerParamList(std::map<Utf8String, Utf8String> &list)
{
//	SquirrelObject arr;

	try 
	{
		Function func(RootTable(vm), "GetServerParamList");

		//SquirrelFunction<SquirrelObject> func(m_Object, _SC("GetServerParamList"));
		if(func.IsNull()) return -1;
		/*Table arr= func.Evaluate<Table>();
		if(arr.IsNull())
			MessageBoxA(0,"olol",0,0);*/

		/*list.clear();
		SquirrelObject key, value;
		arr.BeginIteration();
		while(arr.Next(key, value))
		{
			Utf8String title = value.ToString();
			list[ key.ToString()] =  title;
		}
		arr.EndIteration();*/
	}
	catch (Exception & e) {
		Log(mtError, "CScriptUploadEngine::getServerParamList\r\n" + std::string(e.Message()));
	} 
	FlushSquirrelOutput();
	return 1;

}

int   CScriptUploadEngine::modifyFolder(CFolderItem &folder)
{
	try {
		Function func(RootTable(vm), "ModifyFolder");
		//SquirrelFunction<int> func(m_Object, _SC("ModifyFolder"));
		if(func.IsNull()) return -1;
		int ival = func.Evaluate<int>(&folder);
	}
	catch (Exception & e) {
		Log(mtError, "CScriptUploadEngine::getServerParamList\r\n"+ Utf8String(e.Message()));
	} 
	FlushSquirrelOutput();
	return 1;
}

int CScriptUploadEngine::getFolderList(CFolderList &FolderList)
{
	int ival = 0;
	try
	{
		Function func(RootTable(vm), "GetFolderList");
		//SquirrelFunction<int> func(m_Object, _SC("GetFolderList"));
		if(func.IsNull()) return -1;
		ival = func.Evaluate<int>(&FolderList);
	}
	catch (Exception & e)
	{
		Log(mtError, "CScriptUploadEngine::getFolderList\r\n" + Utf8String(e.Message()));
	} 
	FlushSquirrelOutput();
	return ival;
}

bool CScriptUploadEngine::isLoaded()
{
	return m_bIsPluginLoaded;
}

Utf8String CScriptUploadEngine::name()
{
	return m_sName;
}

int  CScriptUploadEngine::createFolder(CFolderItem &parent, CFolderItem &folder)
{
	int ival = 0;
	try
	{
		Function func(RootTable(vm), "CreateFolder");
		//SquirrelFunction<int> func(m_Object, _SC("CreateFolder"));
		if(func.IsNull()) return -1;
		ival = func.Evaluate<int>(&parent,&folder); // Argument coun*/
	}
	catch (Exception & e)
	{
		Log(mtError, "CScriptUploadEngine::createFolder\r\n" + Utf8String(e.Message()));
	} 
	FlushSquirrelOutput();
	return ival;	
} 

DWORD CScriptUploadEngine::getCreationTime()
{
	return m_CreationTime;
}

void CScriptUploadEngine::setNetworkManager(NetworkManager* nm)
{
	CAbstractUploadEngine::setNetworkManager(nm);
	RootTable(vm).SetValue("nm", nm);
	//BindVariable(m_Object, nm, "nm");
}

bool CScriptUploadEngine::supportsSettings()
{
	//SquirrelObject arr;

	try {
		Function func(RootTable(vm), "GetServerParamList");
		//SquirrelFunction<SquirrelObject> func(m_Object, _SC("GetServerParamList"));
		if(func.IsNull()) return false;
	}
	catch (Exception & e) {
		Log(mtError, "CScriptUploadEngine::supportsSettings\r\n" + std::string(e.Message()));
		return false;
	} 
	//FlushSquirrelOutput();
	return true;

}

int CScriptUploadEngine::RetryLimit()
{
	return 3;
}
