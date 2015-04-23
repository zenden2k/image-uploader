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
#include "Core/Squirrelnc.h"
#include <sqstdaux.h>

#include "Core/ScriptAPI/ScriptAPI.h"
#include "Core/Utils/CryptoUtils.h"

#include "Core/Upload/FileUploadTask.h"
#include "Core/Upload/UrlShorteningTask.h"
#include "Core/Utils/StringUtils.h"
#include "Core/Logging.h"
#ifndef _WIN32
#include <unistd.h>
#endif
#include "Core/ScriptAPI/ScriptAPI.h"
#include <thread>
#include <unordered_map>
const Utf8String IuNewFolderMark = "_iu_create_folder_";

void CScriptUploadEngine::InitScriptEngine()
{
    sqstd_seterrorhandlers(vm_.GetVM());
	ScriptAPI::SetPrintCallback(vm_, ScriptAPI::PrintCallback(this, &CScriptUploadEngine::PrintCallback));
}

void CScriptUploadEngine::DestroyScriptEngine()
{
    ScriptAPI::CleanUp();
}

void CScriptUploadEngine::FlushSquirrelOutput()
{
	ScriptAPI::FlushSquirrelOutput(vm_);
}

void CScriptUploadEngine::PrintCallback(const std::string& output)
{
	std::string taskName;
	if (currentTask_)
	{
		taskName = currentTask_->toString();
	}
	std::thread::id threadId = std::this_thread::get_id();
	Log(ErrorInfo::mtWarning, m_sName + ".nut [Task=" + taskName + ", ThreadId="  + IuCoreUtils::ThreadIdToString(threadId) + "]\r\n" + /*IuStringUtils::ConvertUnixLineEndingsToWindows*/(output));
}
int CScriptUploadEngine::doUpload(UploadTask* task, CIUUploadParams &params)
{
	//std::thread::id threadId = std::this_thread::get_id();
	//LOG(INFO) << "CScriptUploadEngine::doUpload this=" << this << " thread=" << threadId;
    using namespace Sqrat;
	std::string FileName;

	currentTask_ = task;
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
    clearSqratError();
	try
	{
		SetStatus(stUploading);
		if ( task->getType() == "file" ) {
            Function func(vm_.GetRootTable(), "UploadFile");
            if ( func.IsNull() ) {
                 Log(ErrorInfo::mtError, "CScriptUploadEngine::uploadFile\r\n" + std::string("Function UploadFile not found in script")); 
                 clearSqratError();
				 currentTask_ = 0;
                 return -1;
            }
			std::string fname = FileName;
			/*SharedPtr<int> ivalPtr *=*/ ival = ScriptAPI::GetValue(func.Evaluate<int>(fname.c_str(), &params));
            /*if ( ivalPtr ) {
                ival = *ivalPtr.Get();
            }*/
		} else if ( task->getType() == "url" ) {
			UrlShorteningTask *urlShorteningTask = static_cast<UrlShorteningTask*>(task);
            Function func(vm_.GetRootTable(), "ShortenUrl");
            if ( func.IsNull() ) {
                 Log(ErrorInfo::mtError, "CScriptUploadEngine::uploadFile\r\n" + std::string("Function ShortenUrl not found in script")); 
                 clearSqratError();
				 currentTask_ = 0;
                 return -1;
            }
			std::string url = urlShorteningTask->getUrl();
			/*SharedPtr<int> ivalPtr*/ival = ScriptAPI::GetValue(func.Evaluate<int>(url.c_str(), &params));
            if ( ival > 0 ) {
                ival = ival && !params.DirectUrl.empty();
            }
           /* if ( ivalPtr ) {
                ival = *ivalPtr.Get() && !params.DirectUrl.empty();
            }*/
		}
	}
    catch (std::exception & e)
	{
		Log(ErrorInfo::mtError, "CScriptUploadEngine::uploadFile\r\n" + Utf8String(e.what()));
	}
    /*if ( Error::Occurred(vm_.GetVM() ) ) {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::uploadFile\r\n" + Utf8String(Error::Message(vm_.GetVM()))); 
        return false;
    }*/
	
	FlushSquirrelOutput();
	currentTask_ = 0;
	return ival;
}

bool CScriptUploadEngine::needStop()
{
	bool shouldStop = false;
	if (onNeedStop)
		shouldStop = onNeedStop();  // delegate call
	return shouldStop;
}

CScriptUploadEngine::CScriptUploadEngine(Utf8String pluginName, ServerSync* serverSync) : CAbstractUploadEngine(serverSync)
{
	m_sName = pluginName;
	m_CreationTime = time(0);
    m_SquirrelScript = 0;
}

CScriptUploadEngine::~CScriptUploadEngine()
{
    delete m_SquirrelScript;
	//std::thread::id threadId = std::this_thread::get_id();
	//LOG(INFO) << __FUNCTION__ << " this="/*"RemoveFromObjectTable " */ << this << " thread=" << threadId;
	// SquirrelVM::Shutdown();
}

bool CScriptUploadEngine::load(Utf8String fileName, ServerSettingsStruct& params)
{
    using namespace Sqrat;
	if (!IuCoreUtils::FileExists(fileName))
		return false;

	using namespace ScriptAPI;
	setServerSettings(params);
	try
	{   
        InitScriptEngine();

        ScriptAPI::RegisterAPI(vm_);
       
		ServerSettingsStruct* par = &params;
		Sqrat::RootTable& rootTable = vm_.GetRootTable();
		rootTable.SetInstance("ServerParams", par);
		rootTable.SetInstance("serverSync", serverSync_);

		//BindVariable(m_Object, &params, "ServerParams");

		std::string scriptText;
        if ( !IuCoreUtils::ReadUtf8TextFile(fileName, scriptText) ) {
            LOG(ERROR) << "Failed to read script from file " << fileName;
            return false;
        }
        clearSqratError();
        m_SquirrelScript = new Sqrat::Script(vm_.GetVM());
        m_SquirrelScript->CompileString(scriptText.c_str(),IuCoreUtils::ExtractFileName(fileName).c_str());
        /*if ( Error::Occurred(vm_.GetVM() ) ) {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::load failed\r\n" + Utf8String(Error::Message(vm_.GetVM()))); 
			FlushSquirrelOutput();
            return false;
        }*/
        clearSqratError();
        m_SquirrelScript->Run();

        /*if ( Error::Occurred(vm_.GetVM() ) ) {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::load failed\r\n" + Utf8String(Error::Message(vm_.GetVM())));
			FlushSquirrelOutput();
            return false;
        }*/
		ScriptAPI::RegisterShortTranslateFunctions(vm_);
		/*m_SquirrelScript = SquirrelVM::CompileBuffer(scriptText.c_str(), IuCoreUtils::ExtractFileName(fileName).c_str());
		SquirrelVM::RunScript(m_SquirrelScript, &m_Object);*/

		
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
         clearSqratError();
        Function func(vm_.GetRootTable(), "GetFolderAccessTypeList");
        if (func.IsNull()) {
            clearSqratError();
			return -1;
        }
        SharedPtr<Sqrat::Array> arr = func.Evaluate<Sqrat::Array>();

        /*if ( Error::Occurred(vm_.GetVM() ) ) {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::getAccessTypeList\r\n" + Utf8String(Error::Message(vm_.GetVM()))); 
            return 0;
        }*/

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
    Sqrat::Table arr;

	try
	{
        clearSqratError();
        Function func(vm_.GetRootTable(), "GetServerParamList");
        if (func.IsNull()) {
            clearSqratError();
			return -1;
        }
        SharedPtr<Table> arr = func.Evaluate<Sqrat::Table>();

        /*if ( Error::Occurred(vm_.GetVM() ) ) {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::getServerParamList\r\n" + Utf8String(Error::Message(vm_.GetVM()))); 
            return 0;
        }*/
		if (!arr)
		{
			Log(ErrorInfo::mtError, "CScriptUploadEngine::getServerParamList\r\n" + Utf8String("GetServerParamList result is NULL"));

			clearSqratError();
			return -1;
		}

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
        clearSqratError();
        Function func(vm_.GetRootTable(), "DoLogin");
        if (func.IsNull()) {
            clearSqratError();
			return 0;
        }
		int res = ScriptAPI::GetValue(func.Evaluate<int>());
        /*if ( Error::Occurred(vm_.GetVM() ) ) {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::doLogin\r\n" + Utf8String(Error::Message(vm_.GetVM()))); 
            return 0;
        }*/
        return res;
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
        clearSqratError();
        Function func(vm_.GetRootTable(), "ModifyFolder");
        if (func.IsNull()) {
	        clearSqratError();
        }
		res = ScriptAPI::GetValue(func.Evaluate<int>(&folder));
        /*if ( Error::Occurred(vm_.GetVM() ) ) {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::doLogin\r\n" + Utf8String(Error::Message(vm_.GetVM()))); 
            return 0;
        }*/
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
        clearSqratError();
        Function func(vm_.GetRootTable(), "GetFolderList");
        if (func.IsNull()) {
            clearSqratError();
			return -1;
        }
		ival = ScriptAPI::GetValue(func.Evaluate<int>(&FolderList));
        /*if ( Error::Occurred(vm_.GetVM() ) ) {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::getFolderList\r\n" + Utf8String(Error::Message(vm_.GetVM()))); 
        }*/
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
        clearSqratError();
        Function func(vm_.GetRootTable(), "CreateFolder");
        if (func.IsNull()) {
            clearSqratError();
            return -1;
        }
			
		ival = ScriptAPI::GetValue(func.Evaluate<int>(&parent, &folder));
        /*if ( Error::Occurred(vm_.GetVM() ) ) {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::createFolder\r\n" + Utf8String(Error::Message(vm_.GetVM()))); 
            return false;
        }*/
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
    vm_.GetRootTable().SetInstance("nm", nm);
	//BindVariable(m_Object, nm, "nm");
}

bool CScriptUploadEngine::supportsSettings()
{
    using namespace Sqrat;

	try
	{
        clearSqratError();
        Function func(vm_.GetRootTable(), "GetServerParamList");
        if (func.IsNull()) {
            clearSqratError();
			return false;
        }

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
         clearSqratError();
         Function func(vm_.GetRootTable(), "DoLogin");
         if (func.IsNull()) {
            clearSqratError();
			return false;
         }
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

Sqrat::SqratVM& CScriptUploadEngine::getVM()
{
	return vm_;
}

void CScriptUploadEngine::stop()
{
	ScriptAPI::StopAssociatedBrowsers(vm_);
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

void CScriptUploadEngine::clearSqratError()
{
	//SQCLEAR(vm_.GetVM());
}
