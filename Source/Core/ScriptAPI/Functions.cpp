/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#include "Functions.h"

#include "Core/AppParams.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Squirrelnc.h"
#include "Core/Logging.h"
#include <json/json.h>
#include "Core/Network/NetworkClient.h"
#ifdef _WIN32
    #include <windows.h>
    #ifndef IU_CLI
        #include "Gui/Dialogs/InputDialog.h"
        #include "Func/Common.h"
        #include "Func/IuCommonFunctions.h"
        #include "Func/LangClass.h"
		#include "Func/WinUtils.h"
        #include "Gui/Dialogs/LogWindow.h"
    #endif
#endif

#include "Core/Utils/StringUtils.h"
#include "Core/Utils/CryptoUtils.h"
#include <json/json.h>
#include <sstream>
#include <iomanip>
#include <math.h>
#include "Core/3rdpart/CP_RSA.h"
#include "Core/3rdpart/base64.h"
#include "Core/3rdpart/codepages.h"
#include "versioninfo.h"
#include "ScriptAPI.h"

using namespace Sqrat;

namespace ScriptAPI {
/*
SquirrelObject* RootTable = 0;*/

const std::string GetScriptsDirectory()
{
	return AppParams::instance()->settingsDirectory() + "Scripts/";
}

const std::string GetAppLanguageFile()
{
	std::string languageFile = AppParams::instance()->languageFile();
	if ( languageFile.empty() ) {
		return "English";
	}
	return IuCoreUtils::ExtractFileNameNoExt(languageFile);
}

Sqrat::Table GetAppVersion() {
	Sqrat::Table res;
	std::string ver = _APP_VER;
	std::vector<std::string> tokens;
	IuStringUtils::Split(ver,".", tokens, 3);
	if ( tokens.size() >=3 ) {
		res.SetValue("Major", (SQInteger)IuCoreUtils::stringToint64_t(tokens[0]));
		res.SetValue("Minor", (SQInteger)IuCoreUtils::stringToint64_t(tokens[1]));
		res.SetValue("Release", (SQInteger)IuCoreUtils::stringToint64_t(tokens[2]));
		res.SetValue("Build", (SQInteger)IuCoreUtils::stringToint64_t(BUILD));
		bool isGui = 
#ifndef IU_CLI
			true;
#else 
			false;
#endif
		res.SetValue("Gui",isGui);
	}
	return res;
}

Sqrat::Object IncludeScript(const std::string& filename)
{	
	if ( filename.empty() ) {
		LOG(ERROR) << "include() failed: empty file name";
		return Sqrat::Object();
	}
	std::string absolutePath;
	if ( filename[0] == '/' || (filename.length()>1 && filename[1]==':' ) ) {
		absolutePath = filename;
	} else {
		absolutePath = GetScriptsDirectory() + filename;
	}

	if ( !IuCoreUtils::FileExists(absolutePath) ) {
		LOG(ERROR) << "include() failed: file \"" + absolutePath + "\" not found.";
		return Sqrat::Object();
	}
	std::string scriptText;
	if ( !IuCoreUtils::ReadUtf8TextFile(absolutePath, scriptText) ) {
		LOG(ERROR) << "include() failed: could not read file \"" + absolutePath + "\".";
		return Sqrat::Object();
	}
    Sqrat::Script squirrelScript(GetCurrentThreadVM().GetVM());
    squirrelScript.CompileString(scriptText.c_str(),IuCoreUtils::ExtractFileName(absolutePath).c_str());
    squirrelScript.Run();
	return Sqrat::Object();
}

Json::Value* translationRoot = 0;

void CleanUpFunctions() {
	delete translationRoot;
    translationRoot = 0;
}

bool LoadScriptTranslation() {
	if ( !translationRoot ) {
		translationRoot = new Json::Value();
		std::string absolutePath = GetScriptsDirectory() + "Lang/" + GetAppLanguageFile() + ".json";
		std::string jsonText;
		if ( !IuCoreUtils::ReadUtf8TextFile(absolutePath, jsonText) ) {
			return false;
		}
		
		Json::Reader reader;
		if ( reader.parse(jsonText, *translationRoot, false) ) {
			return true;
		}
		return false;
	} else {
		return true;
	}
}

const std::string Translate(const std::string& key, const std::string& originalText) {
	if ( LoadScriptTranslation() ) {
		std::vector<std::string> tokens;
		IuStringUtils::Split(key, ".", tokens, -1);
		Json::Value root = *translationRoot;
		int count = tokens.size();
		for ( int i = 0; i < count; i++ ) {
			std::string token = tokens[i];
			if ( !root.isMember(token) ) {
				break;
			}
			root = root[token];
			if ( root.type() != Json::objectValue && i+1 != count ) {
				break;
			}
			if ( i+1 == count && root.type() == Json::stringValue  ) {
				return root.asString();
			}
		}
	}

#ifndef IU_CLI
	return IuCoreUtils::WstringToUtf8((LPCTSTR)Lang.GetString(IuCoreUtils::Utf8ToWstring(originalText).c_str()));
#endif
	return originalText;
}


#ifdef _WIN32
#ifndef IU_CLI

const std::string Impl_AskUserCaptcha(NetworkClient *nm, const std::string& url)
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
    return ShellExecute(0, _T("open"), IuCoreUtils::Utf8ToWstring(url).c_str(), NULL, NULL, SW_SHOWNORMAL)!=0;
#else
#ifdef __APPLE__
    return system(("open \""+url+"\"").c_str());
#else
    return system(("xdg-open \""+url+"\" >/dev/null 2>&1 & ").c_str());
#endif
#endif
}

const std::string AskUserCaptcha(NetworkClient* nm, const std::string& url)
{
#ifndef IU_CLI
    return Impl_AskUserCaptcha(nm, url);
#else
    ShellOpenUrl(url);
    std::cerr << "Enter text from the image:"<<std::endl;
#ifdef _WIN32
    std::wstring result;
    std::wcin>>result;
    return IuCoreUtils::WstringToUtf8(result);
#else
    std::string result;
    std::cin>>result;
    return result;
#endif


#endif
    return "";
}

#ifndef IU_CLI
const std::string Impl_InputDialog(const std::string& text, const std::string& defaultValue)
{
    CInputDialog dlg(_T("Image Uploader"), Utf8ToWCstring(text), Utf8ToWCstring(defaultValue));

    if(dlg.DoModal(GetActiveWindow())==IDOK) {
        return IuCoreUtils::WstringToUtf8((const TCHAR*)dlg.getValue());
    }
    return "";
}
#endif

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
	LOG(WARNING) << "AnsiToUtf8 not implemented";
	return str; // FIXME
#endif
}

const std::string scriptUtf8ToAnsi(const std::string& str, int codepage )
{
#ifdef _WIN32
	return IuCoreUtils::Utf8ToAnsi(str, codepage);
#else
	LOG(WARNING) << "Utf8ToAnsi not implemented";
	return str; // FIXME
#endif
}

void scriptWriteLog(const std::string& type, const std::string& message) {
#ifndef IU_CLI
	LogMsgType msgType = logWarning;
	if ( type == "error" ) {
		msgType = logError;
	}
	WriteLog(msgType,_T("Script Engine"),Utf8ToWCstring(message));
#else
	std::cerr << type <<" : ";
	#ifdef _WIN32
		std::wcerr<<IuCoreUtils::Utf8ToWstring(message)<<std::endl;;
	#else
		std::cerr<<IuCoreUtils::Utf8ToSystemLocale(message)<<std::endl;
	#endif
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
    sleep(ceil(msec/1000.0));
#endif
}

/*bool ShowText(const std::string& data) {
	return DebugMessage( data, true );
}*/

const std::string escapeJsonString( const std::string& src) {
	return Json::valueToQuotedString(src.data());
}

const std::string scriptGetTempDirectory() {
#ifdef _WIN32
	#ifndef IU_CLI
		return IuCoreUtils::WstringToUtf8((LPCTSTR)IuCommonFunctions::IUTempFolder);
	#else
	TCHAR ShortPath[1024];
	GetTempPath(ARRAY_SIZE(ShortPath), ShortPath);
	TCHAR TempPath[1024];
	if (!GetLongPathName(ShortPath,TempPath, ARRAY_SIZE(TempPath)) ) {
		lstrcpy(TempPath, ShortPath);
	}
	return IuCoreUtils::WstringToUtf8(TempPath);
	#endif
#else
	return "/var/tmp/";
#endif
}

const std::string url_encode(const std::string &value) {
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex << std::uppercase;

    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
		std::string::value_type c = (*i);

		// Keep alphanumeric and other accepted characters intact
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		escaped << '%' << std::setw(2) << int((unsigned char) c);
	}

	return escaped.str();
}


void DebugMessage(const std::string& msg, bool isResponseBody)
{
#ifndef IU_CLI
	DefaultErrorHandling::DebugMessage(msg,isResponseBody);
#else
#ifdef _WIN32
	std::wcerr<<IuCoreUtils::Utf8ToWstring(msg)<<std::endl;;
#else
	std::cerr<<IuCoreUtils::Utf8ToSystemLocale(msg)<<std::endl;
#endif
    //getc(stdin);
#endif
}

const std::string scriptMessageBox( const std::string& message, const std::string &title,const std::string& buttons , const std::string& type) {
#if defined(_WIN32) && !defined(IU_CLI)
	UINT uButtons = MB_OK;
	if ( buttons == "ABORT_RETRY_IGNORE") {
		uButtons = MB_ABORTRETRYIGNORE;
	} else if ( buttons == "CANCEL_TRY_CONTINUE") {
		uButtons = MB_CANCELTRYCONTINUE;
	} else if ( buttons == "OK_CANCEL") {
		uButtons = MB_OKCANCEL;
	} else if ( buttons == "RETRY_CANCEL") {
		uButtons = MB_RETRYCANCEL;
	} else if ( buttons == "YES_NO") {
		uButtons = MB_YESNO;
	}else if ( buttons == "YES_NO_CANCEL") {
		uButtons = MB_YESNOCANCEL;
	}
	UINT icon = 0;
	if ( type == "EXCLAMATION") {
		icon = MB_ICONEXCLAMATION;
	} else if ( type == "WARNING") {
		icon = MB_ICONWARNING;
	} else if ( type == "INFORMATION") {
		icon = MB_ICONINFORMATION;
	} else if ( type == "QUESTION") {
		icon = MB_ICONQUESTION;
	} else if ( type == "ERROR") {
		icon = MB_ICONERROR;
	} 
	int res = MessageBox(GetActiveWindow(), IuCoreUtils::Utf8ToWstring(message).c_str(),  IuCoreUtils::Utf8ToWstring(title).c_str(), uButtons |icon );
	if ( res == IDABORT ) {
		return "ABORT";
	} else if ( res == IDCANCEL ) {
		return "CANCEL";
	} else if ( res == IDCONTINUE ) {
		return "CONTINUE";
	} else if ( res == IDIGNORE ) {
		return "IGNORE";
	} else if ( res == IDNO ) {
		return "NO";
	} else if ( res == IDOK ) {
		return "OK";
	} else if ( res == IDYES ) {
		return "YES";
	}
	else if ( res == IDRETRY ) {
		return "TRY";
	} else if ( res == IDTRYAGAIN ) {
		return "TRY";
	} 
	return "";
	
#else
	std::cerr<<"----------";
#ifdef _WIN32
	std::wcerr<<IuCoreUtils::Utf8ToWstring(title);
#else
	std::cerr<<IuCoreUtils::Utf8ToSystemLocale(title);
#endif
	std::cerr<<"----------"<<std::endl;
#ifdef _WIN32
	std::wcerr<<IuCoreUtils::Utf8ToWstring(message)<<std::endl;;
#else
	std::cerr<<IuCoreUtils::Utf8ToSystemLocale(message)<<std::endl;;
#endif
	if ( buttons.empty() || buttons == "OK") {
		    getc(stdin);
			return "OK";
	} else {
		

		std::vector<std::string> tokens;
		std::map<char,std::string> buttonsMap;
		IuStringUtils::Split(buttons,"_", tokens,10);
		for(int i = 0; i< tokens.size(); i++ ) {
			if ( i !=0 ) {
				std::cerr<< "/";
			}
			buttonsMap[tokens[i][0]] = tokens[i];
			std::cerr<< "("<<tokens[i][0]<<")"<<IuStringUtils::toLower(tokens[i]).c_str()+1;
		}
		std::cerr<<": ";
		char res;
		std::cin >> res;
		res = toupper(res);
		return buttonsMap[res];
	}

#endif
}

void parseJSONObj(Json::Value root, Sqrat::Array& obj);
void parseJSONObj(Json::Value root, Sqrat::Table& obj);

template<class T,class V> void setObjValues(T key, Json::ValueIterator it, V &obj) {
	using namespace Json;
    Sqrat::Array newArr;
    Sqrat::Table newObj;

	switch(it->type()) {
		case nullValue:
			obj.SetValue(key,Sqrat::Object());
			break;
		case intValue:      ///< signed integer value
			obj.SetValue(key, (INT)it->asInt());
			break;
		case uintValue:     ///< unsigned integer value
			obj.SetValue(key, (INT)it->asInt());
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
            parseJSONObj(*it,newArr);
            obj.SetValue(key, newArr);
            break;   
		case objectValue:
            parseJSONObj(*it,newObj);
			obj.SetValue(key, newObj);
            break;   
	}
}

void parseJSONObj(Json::Value root, Sqrat::Array& obj) {
    Json::ValueIterator it;
    obj = Sqrat::Array (GetCurrentThreadVM().GetVM(), root.size());
    for(it = root.begin(); it != root.end(); ++it) {
        int key = it.key().asInt();
        setObjValues(key, it, obj);
    }
}

void parseJSONObj(Json::Value root, Sqrat::Table& obj) {
    Json::ValueIterator it;
    obj = Sqrat::Table (GetCurrentThreadVM().GetVM());
    for(it = root.begin(); it != root.end(); ++it) {
        std::string key = it.key().asString();
        setObjValues(key.data(), it, obj);
    }
}

Sqrat::Object parseJSONObj(Json::Value root) {
	Json::ValueIterator it;
	//SquirrelObject obj;
	bool isArray = root.isArray();
	
	if ( isArray ) {
        Sqrat::Array obj(GetCurrentThreadVM().GetVM(), root.size());
		for(it = root.begin(); it != root.end(); ++it) {
			int key = it.key().asInt();
			setObjValues(key, it, obj);
		}
        return obj;
	} else {
        Sqrat::Table obj(GetCurrentThreadVM().GetVM());
		for(it = root.begin(); it != root.end(); ++it) {
			std::string key = it.key().asString();
			setObjValues(key.data(), it, obj);
		}
        return obj;
	}
    return Sqrat::Object();
}

Sqrat::Object jsonToSquirrelObject(const std::string& json) {
	Json::Value root;
	Json::Reader reader;
	Sqrat::Object sq;
	if ( reader.parse(json, root, false) ) {
		return parseJSONObj(root);
	}
	return sq;
}

Json::Value sqValueToJson(Sqrat::Object obj ) {
	switch ( obj.GetType() ) {
		case OT_NULL:
			return Json::Value(Json::nullValue);
		case OT_INTEGER:
            return SQINT_TO_JSON_VALUE(obj.Cast<int>());
			break;
		case OT_FLOAT:
			return obj.Cast<float>();
			break;

		case OT_BOOL:
			return obj.Cast<bool>();
			break;

		case OT_STRING:
            return obj.Cast<std::string>();
			break;
	}
	return Json::Value(Json::nullValue);
}
Json::Value sqObjToJson(Sqrat::Object obj ) {
	Json::Value res;
    Sqrat::Object::iterator it;
	switch ( obj.GetType() ) {
			case OT_NULL:
			case OT_INTEGER:
			case OT_FLOAT:
			case OT_BOOL:
			case OT_STRING:
				return sqValueToJson(obj);
				break;
			case OT_TABLE:
				while(obj.Next(it) ) {
					res[it.getName()] = sqObjToJson(Sqrat::Object(it.getValue(),GetCurrentThreadVM().GetVM()));
				}
				return res;
				break;
			case OT_ARRAY: 
                while(obj.Next(it) ) {
					res[Sqrat::Object(it.getKey()).Cast<int>()] = sqObjToJson(Sqrat::Object(it.getValue(), GetCurrentThreadVM().GetVM()));
                }
				return res;
				break;				
	}
	return Json::Value(Json::nullValue);
}

const std::string squirrelObjectToJson(Sqrat::Object  obj) {
	Json::Value root = sqObjToJson(obj);
	Json::StreamWriterBuilder builder;
	builder["commentStyle"] = "None";
	builder["indentation"] = "   ";  // or whatever you like

	return Json::writeString(builder, root);
}


const std::string GetFileContents(const std::string& filename) {
	std::string data;
	IuCoreUtils::ReadUtf8TextFile(filename, data);
	return data;
}

unsigned int ScriptGetFileSize(const std::string& filename) {
	return static_cast<unsigned int>(IuCoreUtils::getFileSize(filename));
}

double ScriptGetFileSizeDouble(const std::string& filename) {
	return static_cast<double>(IuCoreUtils::getFileSize(filename));
}

const std::string scriptGetAppLanguage() {
#ifndef IU_CLI
	return IuCoreUtils::WstringToUtf8((LPCTSTR)Lang.getLanguage());
#else 
	return "en";
#endif 
}

const std::string scriptGetAppLocale() {
#ifndef IU_CLI
	return IuCoreUtils::WstringToUtf8((LPCTSTR)Lang.getLocale());
#else 
	return "en_US";
#endif 
}

// older versions of Squirrel Standart Library have broken srand() function
int pluginRandom()
{
    return rand();
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
/*
void DebugMessage(const std::string& message, bool isServerResponseBody)
{
#ifdef IU_CLI
	#ifdef _WIN32
		std::wcerr << IuCoreUtils::Utf8ToWstring(message);
	#else
		std::cerr << IuCoreUtils::Utf8ToSystemLocale(message);
	#endif
#else
	DefaultErrorHandling::DebugMessage(message, isServerResponseBody);
#endif
}
*/
void RegisterFunctions(Sqrat::SqratVM& vm)
{
    Sqrat::RootTable& root = vm.GetRootTable();
	/*RootTable = rootTable;*/
    root
        .Func("GetScriptsDirectory", GetScriptsDirectory)
        .Func("GetAppLanguageFile", GetAppLanguageFile)
        .Func("include", IncludeScript)
        .Func("Translate", Translate)
        .Func("GetAppVersion", GetAppVersion)

        .Func("random", pluginRandom)
        .Func("sleep", scriptSleep)
        .Func("md5", scriptMD5)
        .Func("AnsiToUtf8", scriptAnsiToUtf8)
        .Func("YandexRsaEncrypter", YandexRsaEncrypter)
        .Func("Utf8ToAnsi", scriptUtf8ToAnsi)
        .Func("ExtractFileName", plugExtractFileName)
        .Func("GetFileExtension", plugGetFileExtension)
        .Func("AskUserCaptcha", AskUserCaptcha)
        .Func("InputDialog", InputDialog)
        .Func("GetFileMimeType", scriptGetFileMimeType)
        .Func("JsonEscapeString", escapeJsonString)
        .Func("ShellOpenUrl", ShellOpenUrl)
        .Func("ExtractFileNameNoExt", IuCoreUtils::ExtractFileNameNoExt)
        .Func("ExtractFilePath", IuCoreUtils::ExtractFilePath)
        .Func("ParseJSON", jsonToSquirrelObject)
        .Func("ToJSON", squirrelObjectToJson)
        .Func("CopyFile", IuCoreUtils::copyFile)
        .Func("CreateDirectory", IuCoreUtils::createDirectory)
        .Func("FileExists", IuCoreUtils::FileExists)
        .Func("GetFileContents", GetFileContents)
        .Func("GetTempDirectory", scriptGetTempDirectory)
        .Func("MoveFileOrFolder", IuCoreUtils::MoveFileOrFolder)
        .Func("PutFileContents", IuCoreUtils::PutFileContents)
        .Func("DeleteFile", IuCoreUtils::RemoveFile)
        .Func("GetAppLanguage", scriptGetAppLanguage)
        .Func("GetAppLocale", scriptGetAppLocale)

        .Func("GetFileSize", ScriptGetFileSize)	
        .Func("GetFileSizeDouble", ScriptGetFileSizeDouble)	
        .Func("WriteLog", scriptWriteLog)	
        .Func("MessageBox", scriptMessageBox);	

    using namespace IuCoreUtils;
    root
        .Func("md5_file", &CryptoUtils::CalcMD5HashFromFile)
        .Func("sha1", &CryptoUtils::CalcSHA1HashFromString)
        .Func("sha1_file", &CryptoUtils::CalcSHA1HashFromFile)
        .Func("hmac_sha1", &CryptoUtils::CalcHMACSHA1HashFromString)
        .Func("url_encode", url_encode)
        .Func("DebugMessage", DebugMessage);
    srand(static_cast<unsigned int>(time(0)));

    
}
//	atexit(&CleanUpFunctions);


void RegisterShortTranslateFunctions(Sqrat::SqratVM& vm) {
    Sqrat::RootTable& root = vm.GetRootTable();

    Function func(root, "tr");
    if ( func.IsNull() ) {
        root.Func("tr", Translate);
    }
}

}