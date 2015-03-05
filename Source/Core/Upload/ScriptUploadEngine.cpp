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

#include <openssl/md5.h>
#include <sqplus.h>
#include <sqstdsystem.h>
#include <sstream>
#include <string>
#include <iomanip>
#include <json/json.h>
#include "Core/3rdpart/CP_RSA.h"
#include "Core/3rdpart/base64.h"
#include "Core/3rdpart/codepages.h"

#include "Core/Utils/CryptoUtils.h"
#ifndef IU_CLI
#include "Gui/Dialogs/LogWindow.h"
#endif
#include <Core/Upload/FileUploadTask.h>
#include <Core/Upload/UrlShorteningTask.h>
#include <sstream>

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace SqPlus;
// Squirrel types should be defined in the same module where they are used
// otherwise we will catch SqPlus exception while executing Squirrel functions
///DECLARE_INSTANCE_TYPE(std::string);
DECLARE_INSTANCE_TYPE(ServerSettingsStruct);
DECLARE_INSTANCE_TYPE(NetworkManager);
DECLARE_INSTANCE_TYPE(CFolderList);
DECLARE_INSTANCE_TYPE(CFolderItem);
DECLARE_INSTANCE_TYPE(CIUUploadParams);

#ifdef _WIN32
#ifndef IU_CLI
#include <Gui/Dialogs/InputDialog.h>
#include <Func/Common.h>
#include <Func/IuCommonFunctions.h>
const std::string Impl_AskUserCaptcha(NetworkManager *nm, const std::string& url)
{
	CString wFileName = GetUniqFileName(IuCommonFunctions::IUTempFolder+Utf8ToWstring("captcha").c_str());

	nm->setOutputFile(IuCoreUtils::WstringToUtf8((const TCHAR*)wFileName));
	if(!nm->doGet(url))
		return "";
	CInputDialog dlg(_T("Image Uploader"), TR("¬ведите текст с картинки:"), CString(IuCoreUtils::Utf8ToWstring("").c_str()),wFileName);
	nm->setOutputFile("");
	if(dlg.DoModal()==IDOK)
		return IuCoreUtils::WstringToUtf8((const TCHAR*)dlg.getValue());
	return "";
}
#endif
#endif

bool ShellOpenUrl(const std::string& url) {
#ifdef _WIN32
    return ShellExecute(0, _T("open"), IuCoreUtils::Utf8ToWstring(url).c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
#ifdef __APPLE__
    return system(("open \""+url+"\"").c_str());
#else
    return system(("xdg-open \""+url+"\" >/dev/null 2>&1 & ").c_str());
#endif
#endif
}

const std::string AskUserCaptcha(NetworkManager* nm, const std::string& url)
{
#ifndef IU_CLI
	return Impl_AskUserCaptcha(nm, url);
#else
	ShellOpenUrl(url);
		std::cerr << "Enter text from the image:"<<std::endl;
		std::string result;
		std::cin>>result;
		return result;
	
#endif
	return "";
}



const std::string InputDialog(const std::string& text, const std::string& defaultValue)
{
#ifndef IU_CLI
	return Impl_InputDialog(text, defaultValue);
#else
	std::string result;
	std::cerr<<std::endl<<text<<std::endl;
	std::cin>>result;
	return result;
#endif
	return "";
}


void CFolderList::AddFolder(const std::string& title, const std::string& summary, const std::string& id,
                            const std::string& parentid,
                            int accessType)
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

// older versions of Squirrel Standart Library have broken srand() function
int pluginRandom()
{
	return rand();
}

std::string squirrelOutput;
const Utf8String IuNewFolderMark = "_iu_create_folder_";
static void printFunc(HSQUIRRELVM v, const SQChar* s, ...)
{
	va_list vl;
	va_start(vl, s);
	int len = 1024; // _vcsprintf( s,vl ) + 1;
	char* buffer = new char [len + 1];
	vsnprintf( buffer, len, s, vl);
	va_end(vl);
	// std::wstring text =  Utf8ToWstring(buffer);
	squirrelOutput += buffer;
	delete[] buffer;
}

void CScriptUploadEngine::InitScriptEngine()
{
	SquirrelVM::Init();
	SquirrelVM::PushRootTable();
	sqstd_register_systemlib( SquirrelVM::GetVMPtr() );
}

void CScriptUploadEngine::DestroyScriptEngine()
{
	SquirrelVM::Shutdown();
}

void CScriptUploadEngine::FlushSquirrelOutput()
{
	if (!squirrelOutput.empty())
	{
		Log(ErrorInfo::mtWarning, "Squirrel\r\n" + squirrelOutput);
		squirrelOutput.clear();
	}
}

bool CScriptUploadEngine::doUpload(UploadTask* task, CIUUploadParams &params)
{
	std::string FileName;

	if ( task->getType() == "file" ) {
		FileName = ((FileUploadTask*)task)->getFileName();
	}
	CFolderItem parent, newFolder = m_ServersSettings.newFolder;
	std::string folderID = m_ServersSettings.params["FolderID"];

	if (folderID == IuNewFolderMark)
	{
		SetStatus(stCreatingFolder, newFolder.title);
		if ( createFolder(parent, newFolder))
		{
			folderID = newFolder.id;
			m_ServersSettings.params["FolderID"] = folderID;
			m_ServersSettings.params["FolderUrl"] = newFolder.viewUrl;
		}
		else
			folderID.clear();
	}

	params.folderId = folderID;

	int ival = 0;
	try
	{
		if ( task->getType() == "file" ) {
			SquirrelFunction<int> func(m_Object, _SC("UploadFile"));
			std::string fname = FileName;
			ival = func(fname.c_str(), &params); // Argument coun*/
		} else if ( task->getType() == "url" ) {
			UrlShorteningTask *urlShorteningTask = static_cast<UrlShorteningTask*>(task);
			SquirrelFunction<int> func(m_Object, _SC("ShortenUrl"));
			std::string url = urlShorteningTask->getUrl();
			ival = func(url.c_str(), &params) && !params.DirectUrl.empty(); // Argument coun*/
		}
	}
	catch (SquirrelError& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::uploadFile\r\n" + Utf8String(e.desc));
	}
	FlushSquirrelOutput();
	return ival != 0;
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
	if (onNeedStop)
		m_bShouldStop = onNeedStop();  // delegate call
	return m_bShouldStop;
}

CScriptUploadEngine::CScriptUploadEngine(Utf8String pluginName) : CAbstractUploadEngine()
{
	m_sName = pluginName;
	m_CreationTime = time(0);
}

CScriptUploadEngine::~CScriptUploadEngine()
{
	// SquirrelVM::Shutdown();
}

const std::string YandexRsaEncrypter(const std::string& key, const std::string& data)
{
	if (key.empty())
		return "";              // otherwise we get segfault
	// std::cout<<"key="<<key<<"  data="<<data<<std::endl;
	CCryptoProviderRSA encrypter;
	encrypter.ImportPublicKey(key.c_str());
	char crypted_data[MAX_CRYPT_BITS / sizeof(char)] = "\0";
	size_t crypted_len = 0;
	encrypter.Encrypt(data.c_str(), data.length(), crypted_data, crypted_len);

	return base64_encode((unsigned char*)crypted_data, crypted_len);
}

const std::string scriptGetFileMimeType(const std::string& filename)
{
	return IuCoreUtils::GetFileMimeType(filename);
}

const std::string scriptAnsiToUtf8(const std::string& str, int codepage)
{
#ifdef _WIN32
	return IuCoreUtils::ConvertToUtf8(str, NameByCodepage(codepage));
#else
	return str; // FIXME
#endif
}

const std::string scriptUtf8ToAnsi(const std::string& str, int codepage )
{
#ifdef _WIN32
	return IuCoreUtils::Utf8ToAnsi(str, codepage);
#else
	return str; // FIXME
#endif
}

const std::string scriptMD5(const std::string& data)
{
	return IuCoreUtils::CryptoUtils::CalcMD5HashFromString(data);
}

void scriptSleep(int msec) {
#ifdef _WIN32
	Sleep(msec);
#else
    sleep(msec/1000);
#endif
}

/*bool ShowText(const std::string& data) {
	return DebugMessage( data, true );
}*/

const std::string escapeJsonString( const std::string& src) {
	return Json::valueToQuotedString(src.data());
}

const std::string url_encode(const std::string &value) {
	using namespace std;
	ostringstream escaped;
	escaped.fill('0');
	escaped << hex << std::uppercase;

	for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
		string::value_type c = (*i);

		// Keep alphanumeric and other accepted characters intact
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		escaped << '%' << setw(2) << int((unsigned char) c);
	}

	return escaped.str();
}


void DebugMessage(const std::string& msg, bool isResponseBody)
{
#ifndef IU_CLI
	DefaultErrorHandling::DebugMessage(msg,isResponseBody);
#else
	fprintf(stderr,"%s\r",msg.c_str());
    getc(stdin);
#endif
}

template<class T> void setObjValues(T key, Json::ValueIterator it, SquirrelObject &obj) {
	using namespace Json;

	switch(it->type()) {
		case nullValue:
			obj.SetValue(key,SquirrelObject());
			break;
		case intValue:      ///< signed integer value
			obj.SetValue(key, it->asInt());
			break;
		case uintValue:     ///< unsigned integer value
			obj.SetValue(key, it->asInt());
			break;
		case realValue:
			obj.SetValue(key, it->asFloat());
			break;    ///< double value
		case stringValue:   ///< UTF-8 string value
			obj.SetValue(key, it->asString().data());
			break;   
		case booleanValue:  ///< bool value
			obj.SetValue(key, it->asBool());
			break;   
		case arrayValue:    ///< array value (ordered list)
		case objectValue:
			SquirrelObject newObj;
			obj.SetValue(key, parseJSONObj(*it,newObj));
	}
}

SquirrelObject parseJSONObj(Json::Value root, SquirrelObject &obj) {
	Json::ValueIterator it;
	//SquirrelObject obj;
	bool isArray = root.isArray();
	if ( isArray ) {
		obj = SquirrelVM::CreateArray(root.size());
	} else if ( root.isObject() ) {
		obj = SquirrelVM::CreateTable();
	} 
	

	if ( isArray ) {
		for(it = root.begin(); it != root.end(); ++it) {
			int key = it.key().asInt();
			
			setObjValues(key, it, obj);
		}
	} else {
		for(it = root.begin(); it != root.end(); ++it) {
			std::string key = it.key().asString();
			setObjValues(key.data(), it, obj);
		}
	}
	
	return obj;
}

SquirrelObject jsonToSquirrelObject(const std::string& json) {
	Json::Value root;
	Json::Reader reader;
	SquirrelObject sq;
	if ( reader.parse(json, root, false) ) {
		parseJSONObj(root,sq);
	}
	return sq;
}

consts std::string FileGetContents(const std::string filename) {
	std::string data;
	IuCoreUtils::ReadUtf8TextFile(filename, data);
	return data;
}
bool CScriptUploadEngine::load(Utf8String fileName, ServerSettingsStruct& params)
{
	if (!IuCoreUtils::FileExists(fileName))
		return false;

	setServerSettings(params);
	try
	{
		m_Object = SquirrelVM::CreateTable();

		sq_setprintfunc(SquirrelVM::GetVMPtr(), printFunc /*,printFunc*/);

		SQClassDef<NetworkManager>("NetworkManager").
		func(&NetworkManager::doGet, "doGet").
		func(&NetworkManager::responseBody, "responseBody").
		func(&NetworkManager::responseCode, "responseCode").
		func(&NetworkManager::setUrl, "setUrl").
		func(&NetworkManager::doPost, "doPost").
		func(&NetworkManager::addQueryHeader, "addQueryHeader").
		func(&NetworkManager::addQueryParam, "addQueryParam").
		func(&NetworkManager::addQueryParamFile, "addQueryParamFile").
		func(&NetworkManager::responseHeaderCount, "responseHeaderCount").
		func(&NetworkManager::urlEncode, "urlEncode").
		func(&NetworkManager::errorString, "errorString").
		func(&NetworkManager::doUpload, "doUpload").
		func(&NetworkManager::setMethod, "setMethod").
		func(&NetworkManager::setCurlOption, "setCurlOption").
		func(&NetworkManager::setCurlOptionInt, "setCurlOptionInt").
		func(&NetworkManager::doUploadMultipartData, "doUploadMultipartData").
		func(&NetworkManager::setReferer, "setReferer");


		SQClassDef<CFolderList>("CFolderList").
		func(&CFolderList::AddFolder, "AddFolder").
		func(&CFolderList::AddFolderItem, "AddFolderItem");

		SQClassDef<ServerSettingsStruct>("ServerSettingsStruct").
		func(&ServerSettingsStruct::setParam, "setParam").
		func(&ServerSettingsStruct::getParam, "getParam");

		SQClassDef<CIUUploadParams>("CIUUploadParams").
		func(&CIUUploadParams::getFolderID, "getFolderID").
		func(&CIUUploadParams::setDirectUrl, "setDirectUrl").
		func(&CIUUploadParams::setThumbUrl, "setThumbUrl").
		func(&CIUUploadParams::getServerFileName, "getServerFileName").
		func(&CIUUploadParams::setViewUrl, "setViewUrl").
		func(&CIUUploadParams::getParam, "getParam");

		SQClassDef<CFolderItem>("CFolderItem").
		func(&CFolderItem::getId, "getId").
		func(&CFolderItem::getParentId, "getParentId").
		func(&CFolderItem::getSummary, "getSummary").
		func(&CFolderItem::getTitle, "getTitle").
		func(&CFolderItem::setId, "setId").
		func(&CFolderItem::setParentId, "setParentId").
		func(&CFolderItem::setSummary, "setSummary").
		func(&CFolderItem::setTitle, "setTitle").
		func(&CFolderItem::getAccessType, "getAccessType").
		func(&CFolderItem::setAccessType, "setAccessType").
		func(&CFolderItem::setItemCount, "setItemCount").
		func(&CFolderItem::setViewUrl, "setViewUrl").
		func(&CFolderItem::getItemCount, "getItemCount");
		//using namespace IuCoreUtils;
		RegisterGlobal(pluginRandom, "random");
		RegisterGlobal(scriptSleep, "sleep");
		RegisterGlobal(scriptMD5, "md5");
		RegisterGlobal(scriptAnsiToUtf8, "AnsiToUtf8");
		RegisterGlobal(YandexRsaEncrypter, "YandexRsaEncrypter");
		RegisterGlobal(scriptUtf8ToAnsi, "Utf8ToAnsi");
		RegisterGlobal(plugExtractFileName, "ExtractFileName");
		RegisterGlobal(plugGetFileExtension, "GetFileExtension");
		RegisterGlobal(AskUserCaptcha, "AskUserCaptcha");
		RegisterGlobal(InputDialog, "InputDialog");
		RegisterGlobal(scriptGetFileMimeType, "GetFileMimeType");
		RegisterGlobal(escapeJsonString, "JsonEscapeString");
		RegisterGlobal(ShellOpenUrl, "ShellOpenUrl");
		RegisterGlobal(IuCoreUtils::ExtractFileNameNoExt, "ExtractFileNameNoExt");
		RegisterGlobal(IuCoreUtils::ExtractFilePath, "ExtractFilePath");
		RegisterGlobal(jsonToSquirrelObject, "parseJSON");
		RegisterGlobal(IuCoreUtils::copyFile, "CopyFile");
		RegisterGlobal(IuCoreUtils::createDirectory, "CreateDirectory");
		RegisterGlobal(IuCoreUtils::FileExists, "FileExists");
		RegisterGlobal(FileGetContents, "FileGetContents");
		


		
		

		using namespace IuCoreUtils;
		RegisterGlobal(&CryptoUtils::CalcMD5HashFromFile, "md5_file");
		RegisterGlobal(&CryptoUtils::CalcSHA1HashFromString, "sha1");
		RegisterGlobal(&CryptoUtils::CalcSHA1HashFromFile, "sha1_file");
		RegisterGlobal(&CryptoUtils::CalcHMACSHA1HashFromString, "hmac_sha1");
		RegisterGlobal(url_encode, "url_encode");
		
		srand(static_cast<unsigned int>(time(0)));

		RegisterGlobal(::DebugMessage, "DebugMessage" );

		BindVariable(m_Object, &params, "ServerParams");

		std::string scriptText;
		IuCoreUtils::ReadUtf8TextFile(fileName, scriptText);
		m_SquirrelScript = SquirrelVM::CompileBuffer(scriptText.c_str(), IuCoreUtils::ExtractFileName(fileName).c_str());
		SquirrelVM::RunScript(m_SquirrelScript, &m_Object);
	}
	catch (SquirrelError& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::Load\r\n" + std::string("Unable to load plugin: ") + e.desc);
		return false;
	}
	FlushSquirrelOutput();
	return true;
}

int CScriptUploadEngine::getAccessTypeList(std::vector<Utf8String>& list)
{
	try
	{
		SquirrelFunction<SquirrelObject> func(m_Object, _SC("GetFolderAccessTypeList"));
		if (func.func.IsNull())
			return -1;
		SquirrelObject arr = func();

		list.clear();
		int count =  arr.Len();
		for (int i = 0; i < count; i++)
		{
			std::string title;
			title = arr.GetString(i);
			list.push_back(title);
		}
	}
	catch (SquirrelError& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::getAccessTypeList\r\n" + std::string(e.desc));
	}
	FlushSquirrelOutput();
	return 1;
}

int CScriptUploadEngine::getServerParamList(std::map<Utf8String, Utf8String>& list)
{
	SquirrelObject arr;

	try
	{
		SquirrelFunction<SquirrelObject> func(m_Object, _SC("GetServerParamList"));
		if (func.func.IsNull())
			return -1;
		arr = func();

		list.clear();
		SquirrelObject key, value;
		arr.BeginIteration();
		while (arr.Next(key, value))
		{
			Utf8String title = value.ToString();
			list[key.ToString()] =  title;
		}
		arr.EndIteration();
	}
	catch (SquirrelError& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::getServerParamList\r\n" + std::string(e.desc));
	}
	FlushSquirrelOutput();
	return 1;
}

int CScriptUploadEngine::modifyFolder(CFolderItem& folder)
{
	try
	{
		SquirrelFunction<int> func(m_Object, _SC("ModifyFolder"));
		if (func.func.IsNull())
			return -1;
		func(&folder);
	}
	catch (SquirrelError& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::getServerParamList\r\n" + Utf8String(e.desc));
	}
	FlushSquirrelOutput();
	return 1;
}

int CScriptUploadEngine::getFolderList(CFolderList& FolderList)
{
	int ival = 0;
	try
	{
		SquirrelFunction<int> func(m_Object, _SC("GetFolderList"));
		if (func.func.IsNull())
			return -1;
		ival = func(&FolderList);
	}
	catch (SquirrelError& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::getFolderList\r\n" + Utf8String(e.desc));
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

int CScriptUploadEngine::createFolder(CFolderItem& parent, CFolderItem& folder)
{
	int ival = 0;
	try
	{
		SquirrelFunction<int> func(m_Object, _SC("CreateFolder"));
		if (func.func.IsNull())
			return -1;
		ival = func(&parent, &folder);
	}
	catch (SquirrelError& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::createFolder\r\n" + Utf8String(e.desc));
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

	try
	{
		SquirrelFunction<SquirrelObject> func(m_Object, _SC("GetServerParamList"));
		if (func.func.IsNull())
			return false;
	}
	catch (SquirrelError& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::supportsSettings\r\n" + std::string(e.desc));
		return false;
	}
	FlushSquirrelOutput();
	return true;
}

int CScriptUploadEngine::RetryLimit()
{
	return m_UploadData->RetryLimit;
}

void CScriptUploadEngine::Log(ErrorInfo::MessageType mt, const std::string& error)
{
	ErrorInfo ei;
	ei.ActionIndex = -1;
	ei.messageType = mt;
	ei.errorType = etUserError;
	ei.error = error;
	ei.sender = "CScriptUploadEngine";
	ErrorMessage(ei);
}

