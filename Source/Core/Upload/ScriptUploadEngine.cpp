/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include "ScriptUploadEngine.h"

#include "Core/Scripting/Squirrelnc.h"
#include "Core/Scripting/API/ScriptAPI.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/Upload/UrlShorteningTask.h"
#include "Core/Logging.h"
#include "Core/Scripting/API/ScriptAPI.h"
#include "Core/Upload/ServerSync.h"
#include "Core/ThreadSync.h"
#include <thread>
#include <unordered_map>



CScriptUploadEngine::CScriptUploadEngine(std::string fileName, ServerSync* serverSync, ServerSettingsStruct* settings) : 
                                            CAdvancedUploadEngine(serverSync, settings), Script(fileName, serverSync, false)
{
    setServerSettings(settings);
    name_ = IuCoreUtils::ExtractFileNameNoExt(fileName);
    load(fileName);
}

CScriptUploadEngine::~CScriptUploadEngine()
{
    delete m_SquirrelScript;
}

void CScriptUploadEngine::PrintCallback(const std::string& output)
{
    std::string taskName;
    if (currentTask_)
    {
        taskName = "Task=" + currentTask_->toString() + ", ";
    }
    std::thread::id threadId = std::this_thread::get_id();
    Log(ErrorInfo::mtWarning, name_ + ".nut [" + taskName + "ThreadId=" + IuCoreUtils::ThreadIdToString(threadId) + "]\r\n" + output);
}

int CScriptUploadEngine::doUpload(std::shared_ptr<UploadTask> task, UploadParams& params)
{
    //LOG(INFO) << "CScriptUploadEngine::doUpload this=" << this << " thread=" << threadId;
    using namespace Sqrat;
    std::string FileName;

    currentTask_ = task;
    if (task->type() == UploadTask::TypeFile ) {
        FileName = (dynamic_cast<FileUploadTask*>(task.get()))->getFileName();
    }
    CFolderItem parent;

    {
        std::string folderID;
        std::lock_guard<std::mutex> guard(serverSync_->folderMutex());
        CFolderItem& newFolder = m_ServersSettings->newFolder;
        if (task->serverProfile().folderId() == CFolderItem::NewFolderMark) {
            task->serverProfile().setFolderId(newFolder.getId());
            task->serverProfile().setFolderTitle(newFolder.getTitle());

            if (newFolder.getId() == CFolderItem::NewFolderMark) {
                SetStatus(stCreatingFolder, newFolder.title);
                if (createFolder(parent, newFolder)) {
                    folderID = newFolder.id;
                    task->serverProfile().setFolderId(folderID);
                    m_ServersSettings->setParam("FolderID", folderID);
                    m_ServersSettings->setParam("FolderUrl", newFolder.viewUrl);
                } else {
                    folderID.clear();
                }
            }
        }
    }
   
    params.folderId = task->serverProfile().folderId();
    params.task_ = task;
    int ival = 0;

    if (!params.folderId.empty() && task->OnFolderUsed) {
        task->OnFolderUsed(task.get());
    }
    try {
        checkCallingThread();
        SetStatus(stUploading);
        if (task->type() == UploadTask::TypeFile) {
            Function func(vm_.GetRootTable(), "UploadFile");
            if ( func.IsNull() ) {
                 Log(ErrorInfo::mtError, "CScriptUploadEngine::uploadFile\r\n" + std::string("Function UploadFile not found in script")); 
                 currentTask_ = nullptr;
                 return -1;
            }
            std::string fname = FileName;
            ival = ScriptAPI::GetValue(func.Evaluate<int>(fname.c_str(), &params));
        } else if (task->type() == UploadTask::TypeUrl ) {
            std::shared_ptr<UrlShorteningTask> urlShorteningTask = std::dynamic_pointer_cast<UrlShorteningTask>(task);
            Function func(vm_.GetRootTable(), "ShortenUrl");
            if ( func.IsNull() ) {
                 Log(ErrorInfo::mtError, "CScriptUploadEngine::uploadFile\r\n" + std::string("Function ShortenUrl not found in script")); 
                 currentTask_ = nullptr;
                 return -1;
            }
            std::string url = urlShorteningTask->getUrl();
            /*SharedPtr<int> ivalPtr*/ival = ScriptAPI::GetValue(func.Evaluate<int>(url.c_str(), &params));
            if ( ival > 0 ) {
                ival = ival && !params.DirectUrl.empty();
            }
        }
    }
    catch (NetworkClient::AbortedException & ex) {
        throw ex;
    }
    catch (ServerSyncException& e) {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::uploadFile\r\n" + std::string(e.what()));
        ival = -1; // fatal error
    }
    catch (std::exception & e) {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::uploadFile\r\n" + std::string(e.what()));
    }
    
    FlushSquirrelOutput();
    currentTask_ = nullptr;
    return ival;
}

bool CScriptUploadEngine::needStop()
{
    bool shouldStop = false;
    if (onNeedStop) {
        shouldStop = onNeedStop();  // delegate call
    }
    return shouldStop;
}

bool CScriptUploadEngine::preLoad()
{
    try {   
        ServerSettingsStruct* par = m_ServersSettings;
        Sqrat::RootTable& rootTable = vm_.GetRootTable();
        rootTable.SetInstance("ServerParams", par);
        rootTable.SetInstance("Sync", serverSync_);
    } catch (std::exception& e) {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::preLoad failed\r\n" + std::string("Error: ") + e.what());
        return false;
    }
    FlushSquirrelOutput();
    return true;
}

bool CScriptUploadEngine::postLoad()
{
    ScriptAPI::RegisterShortTranslateFunctions(vm_);
    return 0;
}

void CScriptUploadEngine::logNetworkError(bool error, const std::string& msg) {
    std::thread::id threadId = std::this_thread::get_id();
    Log(error ? (ErrorInfo::mtError) : (ErrorInfo::mtWarning), name_ + ".nut [" + "ThreadId=" + IuCoreUtils::ThreadIdToString(threadId) + "]\r\n" +msg);
}

int CScriptUploadEngine::getAccessTypeList(std::vector<std::string>& list)
{
    using namespace Sqrat;
    try
    {
        checkCallingThread();

        Function func(vm_.GetRootTable(), "GetFolderAccessTypeList");
        if (func.IsNull()) {

            return -1;
        }
        SharedPtr<Sqrat::Array> arr = func.Evaluate<Sqrat::Array>();

        /*if ( Error::Occurred(vm_.GetVM() ) ) {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::getAccessTypeList\r\n" + std::string(Error::Message(vm_.GetVM()))); 
            return 0;
        }*/

        list.clear();
        auto count =  arr->GetSize();
        for (auto i = 0; i < count; i++)
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

int CScriptUploadEngine::getServerParamList(std::map<std::string, std::string>& list)
{
    using namespace Sqrat;

    try
    {
        checkCallingThread();
        Function func(vm_.GetRootTable(), "GetServerParamList");
        if (func.IsNull()) {
            return -1;
        }
        SharedPtr<Table> arr = func.Evaluate<Sqrat::Table>();

        if (!arr)
        {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::getServerParamList\r\n" + std::string("GetServerParamList result is NULL"));

            return -1;
        }

        Sqrat::Array::iterator it;
        list.clear();
        
        while (arr->Next(it))
        {
            std::string t = Sqrat::Object(it.getValue(), vm_.GetVM()).Cast<std::string>();
            /*if ( t ) */{
                std::string title = t;
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
        checkCallingThread();

        Function func(vm_.GetRootTable(), "DoLogin");
        if (func.IsNull()) {
            return 0;
        }
        int res = ScriptAPI::GetValue(func.Evaluate<int>());
        /*if ( Error::Occurred(vm_.GetVM() ) ) {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::doLogin\r\n" + std::string(Error::Message(vm_.GetVM()))); 
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
        checkCallingThread();
        Function func(vm_.GetRootTable(), "ModifyFolder");
        if (func.IsNull()) {
            return 0;
        }
        res = ScriptAPI::GetValue(func.Evaluate<int>(&folder));
        /*if ( Error::Occurred(vm_.GetVM() ) ) {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::doLogin\r\n" + std::string(Error::Message(vm_.GetVM()))); 
            return 0;
        }*/
    }
    catch (std::exception& e)
    {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::getServerParamList\r\n" + std::string(e.what()));
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
        checkCallingThread();
        Function func(vm_.GetRootTable(), "GetFolderList");
        if (func.IsNull()) {
            return -1;
        }
        ival = ScriptAPI::GetValue(func.Evaluate<int>(&FolderList));
        /*if ( Error::Occurred(vm_.GetVM() ) ) {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::getFolderList\r\n" + std::string(Error::Message(vm_.GetVM()))); 
        }*/
    }
    catch (NetworkClient::AbortedException& ) {
        FlushSquirrelOutput();
        return 0;
    }
    catch (std::exception& e)
    {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::getFolderList\r\n" + std::string(e.what()));
    }
    FlushSquirrelOutput();
    return ival;
}

std::string CScriptUploadEngine::name()
{
    return name_;
}

int CScriptUploadEngine::createFolder(const CFolderItem& parent, CFolderItem& folder)
{
    using namespace Sqrat;
    int ival = 0;
    try
    {
        checkCallingThread();
        Function func(vm_.GetRootTable(), "CreateFolder");
        if (func.IsNull()) {
            return -1;
        }
            
        ival = ScriptAPI::GetValue(func.Evaluate<int>(&parent, &folder));
        /*if ( Error::Occurred(vm_.GetVM() ) ) {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::createFolder\r\n" + std::string(Error::Message(vm_.GetVM()))); 
            return false;
        }*/
    }
    catch (std::exception& e)
    {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::createFolder\r\n" + std::string(e.what()));
    }
    FlushSquirrelOutput();
    return ival;
}

void CScriptUploadEngine::setNetworkClient(NetworkClient* nm)
{
    CAbstractUploadEngine::setNetworkClient(nm);
    if (!m_UploadData->UserAgent.empty()) {
        nm->setUserAgent(m_UploadData->UserAgent);
    }
    nm->setCurlShare(sync_->getCurlShare());
    //nm->setErrorLogId("[" + name_ + ".nut]");
    nm->setLogger(this);
    vm_.GetRootTable().SetInstance("nm", nm);
    //BindVariable(m_Object, nm, "nm");
}

bool CScriptUploadEngine::supportsSettings()
{
    using namespace Sqrat;

    try {
        checkCallingThread();
        Function func(vm_.GetRootTable(), "GetServerParamList");
        if (func.IsNull()) {
            return false;
        }
    } catch (std::exception& e) {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::supportsSettings\r\n" + std::string(e.what()));
        return false;
    }
    FlushSquirrelOutput();
    return true;
}

bool CScriptUploadEngine::supportsBeforehandAuthorization()
{
    using namespace Sqrat;
    try {
        checkCallingThread();
        Function func(vm_.GetRootTable(), "DoLogin");
        if (func.IsNull()) {
            return false;
        }
    } catch (std::exception& e) {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::supportsBeforehandAuthorization\r\n" + std::string(e.what()));
        return false;
    }
    FlushSquirrelOutput();
    return true;
}

void CScriptUploadEngine::stop()
{
    ScriptAPI::StopAssociatedBrowsers(vm_);
    CAdvancedUploadEngine::stop();
}

void CScriptUploadEngine::Log(ErrorInfo::MessageType mt, const std::string& error)
{
    ErrorInfo ei;
    ei.ActionIndex = -1;
    ei.messageType = mt;
    ei.errorType = etUserError;
    ei.error = error;
    ei.sender = "CScriptUploadEngine";
    ei.ServerName = m_UploadData->Name;
    if (currentTask_) {
        ei.FileName = currentTask_->toString();
    }
    
    ErrorMessage(ei);
}
