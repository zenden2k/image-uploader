/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
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


/* This module contains uploading engine which uses SqPlus binding library
 for executing Squirrel scripts */

#include "ScriptUploadEngine.h"
#include <stdarg.h>
#include <iostream>
#ifndef IU_CLI
   #include "../../atlheaders.h"
   #include "../../Func/Common.h"
   #include "../../Gui/Dialogs/InputDialog.h"
	#include "../../Func/LangClass.h"
#endif

#include <openssl/md5.h>
#undef UNICODE
#undef _UNICODE
#include <sqplus.h>
#include <sqstdsystem.h>

#include "../3rdpart/CP_RSA.h"
#include "../3rdpart/base64.h"
#include "../3rdpart/codepages.h"

using namespace SqPlus;
// Squirrel types should be defined in the same module where they are used
// otherwise we will catch SqPlus exception while executing Squirrel functions
DECLARE_INSTANCE_TYPE(ServerSettingsStruct);
DECLARE_INSTANCE_TYPE(NetworkManager);
DECLARE_INSTANCE_TYPE(CFolderList);
DECLARE_INSTANCE_TYPE(CFolderItem);
DECLARE_INSTANCE_TYPE(CIUUploadParams);

const std::string AskUserCaptcha(NetworkManager *nm, const std::string& url)
{
#ifndef IU_CLI
	CString wFileName = GetUniqFileName(IUTempFolder+Utf8ToWstring("captcha").c_str());
	
	nm->setOutputFile(IuCoreUtils::WstringToUtf8((const TCHAR*)wFileName));
	if(!nm->doGet(url))
		return "";
	CInputDialog dlg(_T("Image Uploader"), TR("¬ведите текст с картинки:"), CString(IuCoreUtils::Utf8ToWstring("").c_str()),wFileName);
	nm->setOutputFile("");
	if(dlg.DoModal()==IDOK)
		return IuCoreUtils::WstringToUtf8((const TCHAR*)dlg.getValue());
#else
	return "<not implemented>";
#endif
	return "";
}

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

void CScriptUploadEngine::InitScriptEngine()
{
	SquirrelVM::Init();
	SquirrelVM::PushRootTable();
}

void CScriptUploadEngine::DestroyScriptEngine()
{
	SquirrelVM::Shutdown();
}

void CScriptUploadEngine::FlushSquirrelOutput()
{
	if(!squirrelOutput.empty())
	{
		Log(mtWarning, "Squirrel\r\n" + squirrelOutput);
		squirrelOutput.clear();
	}
}

bool CScriptUploadEngine::doUpload(Utf8String FileName, Utf8String DisplayName, CIUUploadParams &params)
{
	CFolderItem parent, newFolder = m_ServersSettings.newFolder;
	std::string folderID = m_ServersSettings.params["FolderID"];

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
		SquirrelFunction<int> func(m_Object, _SC("UploadFile"));
		std::string fname = FileName;
		ival = func(fname.c_str(), &params); // Argument coun*/
	}
	catch (SquirrelError & e) {
		Log(mtError, "CScriptUploadEngine::uploadFile\r\n" + Utf8String(e.desc));     
	} 
	FlushSquirrelOutput();
	return ival!=0;
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
	m_sName = pluginName;
	m_CreationTime = time(0); 
	
}

CScriptUploadEngine::~CScriptUploadEngine()
{
	//SquirrelVM::Shutdown();
}

const std::string YandexRsaEncrypter(const std::string& key, const std::string& data)
{
	if(key.empty()) return ""; // otherwise we get segfault
	//std::cout<<"key="<<key<<"  data="<<data<<std::endl;
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

const std::string scriptAnsiToUtf8(const std::string &str, int codepage)
{
#ifdef _WIN32
	return IuCoreUtils::ConvertToUtf8(str,NameByCodepage(codepage));
#else
	return str; //FIXME
#endif

}

const std::string scriptUtf8ToAnsi(const std::string &str, int codepage )
{
#ifdef _WIN32
	return IuCoreUtils::ConvertToUtf8(str,NameByCodepage(codepage));
#else
	return str; //FIXME
#endif
}

const std::string scriptMD5(const std::string& data)
{
	return IuCoreUtils::CalcMD5Hash(data);
}

bool CScriptUploadEngine::load(Utf8String fileName, ServerSettingsStruct& params)
{  
	if(!IuCoreUtils::FileExists(fileName))
		return false;

	setServerSettings(params);
	try
	{
	m_Object = SquirrelVM::CreateTable();

	sq_setprintfunc(SquirrelVM::GetVMPtr(),printFunc/*,printFunc*/);
	 
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
			func(&CIUUploadParams::setViewUrl,"setViewUrl").	
			func(&CIUUploadParams::getParam,"getParam");	

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
		RegisterGlobal(scriptMD5, "md5");
		RegisterGlobal(scriptAnsiToUtf8, "AnsiToUtf8");
		RegisterGlobal(YandexRsaEncrypter,"YandexRsaEncrypter");
		RegisterGlobal(scriptUtf8ToAnsi, "Utf8ToAnsi");
		RegisterGlobal(plugExtractFileName, "ExtractFileName");
		RegisterGlobal(plugGetFileExtension, "GetFileExtension");
		RegisterGlobal(AskUserCaptcha, "AskUserCaptcha");
		RegisterGlobal(scriptGetFileMimeType, "GetFileMimeType");
		srand(static_cast<unsigned int>(time(0)));
	
	BindVariable(m_Object, &m_ServersSettings, "ServerParams");

		std::string scriptText;
		IuCoreUtils::ReadUtf8TextFile(fileName, scriptText);
		m_SquirrelScript = SquirrelVM::CompileBuffer(scriptText.c_str(), IuCoreUtils::ExtractFileName(fileName).c_str());
		SquirrelVM::RunScript(m_SquirrelScript, &m_Object);
	}
	catch (SquirrelError & e) {
		Log(mtError, "CScriptUploadEngine::Load\r\n" + std::string("Unable to load plugin: ") + e.desc);
		return false;
	} 
	FlushSquirrelOutput();
	return true;
}

int CScriptUploadEngine::getAccessTypeList(std::vector<Utf8String> &list)
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
			list.push_back(title);
		}
	}
	catch (SquirrelError & e) {
		Log(mtError, "CScriptUploadEngine::getAccessTypeList\r\n"+std::string(e.desc));	
	} 
	FlushSquirrelOutput();
	return 1;

}

int CScriptUploadEngine::getServerParamList(std::map<Utf8String, Utf8String> &list)
{
	SquirrelObject arr;

	try 
	{
		SquirrelFunction<SquirrelObject> func(m_Object, _SC("GetServerParamList"));
		if(func.func.IsNull()) return -1;
		arr = func();

		list.clear();
		SquirrelObject key, value;
		arr.BeginIteration();
		while(arr.Next(key, value))
		{
			Utf8String title = value.ToString();
			list[ key.ToString()] =  title;
		}
		arr.EndIteration();
	}
	catch (SquirrelError & e) {
		Log(mtError, "CScriptUploadEngine::getServerParamList\r\n" + std::string(e.desc));	
	} 
	FlushSquirrelOutput();
	return 1;

}

int CScriptUploadEngine::modifyFolder(CFolderItem &folder)
{
	try {
		SquirrelFunction<int> func(m_Object, _SC("ModifyFolder"));
		if(func.func.IsNull()) return -1;
		func(&folder);
	}
	catch (SquirrelError & e) {
		Log(mtError, "CScriptUploadEngine::getServerParamList\r\n"+ Utf8String(e.desc));	
	} 
	FlushSquirrelOutput();
	return 1;
}

int CScriptUploadEngine::getFolderList(CFolderList &FolderList)
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
		Log(mtError, "CScriptUploadEngine::getFolderList\r\n" + Utf8String(e.desc));	
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
		SquirrelFunction<int> func(m_Object, _SC("CreateFolder"));
		if(func.func.IsNull()) return -1;
		ival = func(&parent,&folder); // Argument coun*/
	}
	catch (SquirrelError & e) 
	{
		Log(mtError, "CScriptUploadEngine::createFolder\r\n" + Utf8String(e.desc));	
	} 
	FlushSquirrelOutput();
	return ival;	
} 

time_t CScriptUploadEngine::getCreationTime()
{
	return m_CreationTime;
}

void CScriptUploadEngine::setNetworkManager(NetworkManager* nm)
{
	CAbstractUploadEngine::setNetworkManager(nm);
	BindVariable(m_Object, nm, "nm");
}

bool CScriptUploadEngine::supportsSettings()
{
	SquirrelObject arr;

	try {
		SquirrelFunction<SquirrelObject> func(m_Object, _SC("GetServerParamList"));
		if(func.func.IsNull()) return false;
	}
	catch (SquirrelError & e) {
		Log(mtError, "CScriptUploadEngine::supportsSettings\r\n" + std::string(e.desc));	
		return false;
	} 
	FlushSquirrelOutput();
	return true;
}

int CScriptUploadEngine::RetryLimit()
{
	return m_UploadData->RetryLimit;
}

void  CScriptUploadEngine::Log(MessageType mt, const std::string& error)
{
	ErrorInfo ei;
	ei.ActionIndex = -1;
	ei.messageType = mt;
	ei.errorType = etUserError;
	ei.error = error;
	ei.sender = "CScriptUploadEngine";
	ErrorMessage(ei);
}

