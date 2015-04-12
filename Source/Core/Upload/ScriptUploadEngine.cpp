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

/* This module contains uploading engine which uses SqPlus binding library
 for executing Squirrel scripts */

#include "ScriptUploadEngine.h"
#include <stdarg.h>
#include <iostream>
#include <Core/Squirrelnc.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdsystem.h>


#include "Core/Utils/CryptoUtils.h"

#include <Core/Upload/FileUploadTask.h>
#include <Core/Upload/UrlShorteningTask.h>
#include <sstream>
#include <Core/Utils/StringUtils.h>
#include <Core/Logging.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <Core/ScriptAPI/ScriptAPI.h>
/*
using namespace SqPlus;
// Squirrel types should be defined in the same module where they are used
// otherwise we will catch SqPlus exception while executing Squirrel functions
///DECLARE_INSTANCE_TYPE(std::string);
using namespace  ScriptAPI;
DECLARE_INSTANCE_TYPE(NetworkClient);
DECLARE_INSTANCE_TYPE(ServerSettingsStruct);
DECLARE_INSTANCE_TYPE(CFolderList);
DECLARE_INSTANCE_TYPE(CFolderItem);
DECLARE_INSTANCE_TYPE(CIUUploadParams);*/

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

void CompilerErrorHandler(HSQUIRRELVM,const SQChar * desc,const SQChar * source,SQInteger line,SQInteger column) {
	LOG(ERROR) << "Script compilation failed\r\n"<<"File:  "<<source<<"\r\nLine:"<<line<<"   Column:"<<column<<"\r\n\r\n"<<desc;
}

void CScriptUploadEngine::InitScriptEngine()
{
    /*// Create a new VM - a root VM with new SharedState.
    vm_ = sq_open(1024);
    if( !vm_ ) return;
    //sq_setprintfunc(v,SquirrelVM::PrintFunc);
    sq_pushroottable(vm_);
    sqstd_register_iolib(vm_);
    sqstd_register_bloblib(vm_);
    sqstd_register_mathlib(vm_);
    sqstd_register_stringlib(vm_); 
    sqstd_register_systemlib(vm_);      
    sqstd_seterrorhandlers(vm_);
    //TODO error handler, compiler error handler
    sq_pop(vm_, 1); */

    sq_setcompilererrorhandler(vm_.GetVM(), CompilerErrorHandler);
    sq_setprintfunc(/*SquirrelVM::GetVMPtr()*/vm_.GetVM(),printFunc,printFunc);
    //sq_pushroottable(vm_);
}

void CScriptUploadEngine::DestroyScriptEngine()
{
	/*SquirrelVM::Shutdown();*/
    ScriptAPI::CleanUp();
}

void CScriptUploadEngine::FlushSquirrelOutput()
{
	if (!squirrelOutput.empty())
	{
		Log(ErrorInfo::mtWarning, "Squirrel\r\n" + /*IuStringUtils::ConvertUnixLineEndingsToWindows*/(squirrelOutput));
		squirrelOutput.clear();
	}
}

int CScriptUploadEngine::doUpload(UploadTask* task, CIUUploadParams &params)
{
    using namespace Sqrat;
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
		SetStatus(stUploading);
		if ( task->getType() == "file" ) {
            Function func(vm_.GetRootTable(), "UploadFile");
			std::string fname = FileName;
			/*SharedPtr<int> ivalPtr *=*/ ival =   func.Evaluate<int>(fname.c_str(), &params);
            /*if ( ivalPtr ) {
                ival = *ivalPtr.Get();
            }*/
		} else if ( task->getType() == "url" ) {
			UrlShorteningTask *urlShorteningTask = static_cast<UrlShorteningTask*>(task);
            Function func(vm_.GetRootTable(), "ShortenUrl");
			std::string url = urlShorteningTask->getUrl();
			/*SharedPtr<int> ivalPtr*/ival  = func.Evaluate<int>(url.c_str(), &params); 
            ival = ival && !params.DirectUrl.empty();;
           /* if ( ivalPtr ) {
                ival = *ivalPtr.Get() && !params.DirectUrl.empty();
            }*/
		}
	}
    catch (std::exception & e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::uploadFile\r\n" + Utf8String(e.what()));
	}
	FlushSquirrelOutput();
	return ival != 0;
}

bool CScriptUploadEngine::needStop()
{
	bool shouldStop = false;
	if (onNeedStop)
		shouldStop = onNeedStop();  // delegate call
	return shouldStop;
}

CScriptUploadEngine::CScriptUploadEngine(Utf8String pluginName) : CAbstractUploadEngine()
{
	m_sName = pluginName;
	m_CreationTime = time(0);
    m_SquirrelScript = 0;
}

CScriptUploadEngine::~CScriptUploadEngine()
{
	// SquirrelVM::Shutdown();
}

bool CScriptUploadEngine::load(Utf8String fileName, ServerSettingsStruct& params)
{
	if (!IuCoreUtils::FileExists(fileName))
		return false;

	using namespace ScriptAPI;
	setServerSettings(params);
	try
	{   
        InitScriptEngine();

        ScriptAPI::RegisterFunctions(vm_.GetVM());
        ScriptAPI::RegisterClasses(vm_.GetVM());

		//BindVariable(m_Object, &params, "ServerParams");

		std::string scriptText;
        if ( !IuCoreUtils::ReadUtf8TextFile(fileName, scriptText) ) {
            LOG(ERROR) << "Failed to read script from file " << fileName;
            return false;
        }
        m_SquirrelScript = new Sqrat::Script(vm_.GetVM());
        m_SquirrelScript->CompileString(scriptText.c_str(),IuCoreUtils::ExtractFileName(fileName).c_str());
        m_SquirrelScript->Run();

		/*m_SquirrelScript = SquirrelVM::CompileBuffer(scriptText.c_str(), IuCoreUtils::ExtractFileName(fileName).c_str());
		SquirrelVM::RunScript(m_SquirrelScript, &m_Object);*/

		//ScriptAPI::RegisterShortTranslateFunctions(!m_Object.Exists("tr"), !m_Object.Exists("__"));
	}
    catch (std::exception& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::Load failed\r\n" + std::string("Error: ") + e.what());
		return false;
	}
	FlushSquirrelOutput();
	return true;
}

int CScriptUploadEngine::getAccessTypeList(std::vector<Utf8String>& list)
{
     using namespace Sqrat;
	try
	{
        Function func(vm_.GetRootTable(), "GetFolderAccessTypeList");
		if (func.IsNull())
			return -1;
        SharedPtr<Sqrat::Object> arr = func.Evaluate<Sqrat::Object>();

		list.clear();
		int count =  arr->GetSize();
		for (int i = 0; i < count; i++)
		{
			std::string title;
            title = arr->GetSlot(i).Cast<std::string>();
			list.push_back(title);
		}
	}
    catch (std::exception& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::getAccessTypeList\r\n" + std::string(e.what()));
	}
	FlushSquirrelOutput();
	return 1;
}

int CScriptUploadEngine::getServerParamList(std::map<Utf8String, Utf8String>& list)
{
    using namespace Sqrat;
    Sqrat::Array arr;

	try
	{
        Function func(vm_.GetRootTable(), "GetServerParamList");
		if (func.IsNull())
			return -1;
        SharedPtr<Array> arr = func.Evaluate<Sqrat::Array>();
        Sqrat::Array::iterator it;
		list.clear();
        
		while (arr->Next(it))
		{
            std::string t = Sqrat::Object(it.getValue(), vm_.GetVM()).Cast<std::string>();
			/*if ( t ) */{
				Utf8String title = t;
				list[it.getName()] =  title;
			}
		}
	}
    catch (std::exception& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::getServerParamList\r\n" + std::string(e.what()));
	}
	FlushSquirrelOutput();
	return 1;
}

int CScriptUploadEngine::doLogin()
{
    using namespace Sqrat;
	try
	{
        Function func(vm_.GetRootTable(), "DoLogin");
		if (func.IsNull())
			return 0;
		return func.Evaluate<int>();
	}
    catch (std::exception& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::doLogin\r\n" + std::string(e.what()));
	}
	FlushSquirrelOutput();
    return 0;
}

int CScriptUploadEngine::modifyFolder(CFolderItem& folder)
{
    using namespace Sqrat;
    int res = 1;
	try
	{
        Function func(vm_.GetRootTable(), "ModifyFolder");
		if (func.IsNull())
			return -1;
		res = func.Evaluate<int>(&folder);
	}
    catch (std::exception& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::getServerParamList\r\n" + Utf8String(e.what()));
	}
	FlushSquirrelOutput();
	return res;
}

int CScriptUploadEngine::getFolderList(CFolderList& FolderList)
{
    using namespace Sqrat;
	int ival = 0;
	try
	{
        Function func(vm_.GetRootTable(), "GetFolderList");
		if (func.IsNull())
			return -1;
		ival = func.Evaluate<int>(&FolderList);
	}
    catch (std::exception& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::getFolderList\r\n" + Utf8String(e.what()));
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
    using namespace Sqrat;
	int ival = 0;
	try
	{
        Function func(vm_.GetRootTable(), "CreateFolder");
		if (func.IsNull())
			return -1;
		ival = func.Evaluate<int>(&parent, &folder);
	}
    catch (std::exception& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::createFolder\r\n" + Utf8String(e.what()));
	}
	FlushSquirrelOutput();
	return ival;
}

time_t CScriptUploadEngine::getCreationTime()
{
	return m_CreationTime;
}

void CScriptUploadEngine::setNetworkClient(NetworkClient* nm)
{
	CAbstractUploadEngine::setNetworkClient(nm);
    vm_.GetRootTable().SetValue("nm", nm);
	//BindVariable(m_Object, nm, "nm");
}

bool CScriptUploadEngine::supportsSettings()
{
    using namespace Sqrat;

	try
	{
        Function func(vm_.GetRootTable(), "GetServerParamList");
		if (func.IsNull())
			return false;
	}
    catch (std::exception& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::supportsSettings\r\n" + std::string(e.what()));
		return false;
	}
	FlushSquirrelOutput();
	return true;
}

bool CScriptUploadEngine::supportsBeforehandAuthorization()
{
    using namespace Sqrat;
	try
	{
         Function func(vm_.GetRootTable(), "DoLogin");
		 if (func.IsNull())
			return false;
	}
    catch (std::exception& e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::supportsBeforehandAuthorization\r\n" + std::string(e.what()));
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

