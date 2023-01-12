/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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

#include <thread>

#include "Core/Scripting/Squirrelnc.h"
#include "Core/Scripting/API/ScriptAPI.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/Upload/UrlShorteningTask.h"
#include "Core/Upload/ServerSync.h"
#include "Core/ThreadSync.h"
#include "AuthTask.h"
#include "FolderTask.h"

CScriptUploadEngine::CScriptUploadEngine(const std::string& fileName, ServerSync* serverSync, ServerSettingsStruct* settings, 
                                         std::shared_ptr<INetworkClientFactory> factory, ErrorMessageCallback errorCallback) :
    CAdvancedUploadEngine(serverSync, settings, std::move(errorCallback)),
    Script(fileName, serverSync, std::move(factory), false)
{
    setServerSettings(settings);
    newAuthMode = false;
    name_ = IuCoreUtils::ExtractFileNameNoExt(fileName);
    load(fileName);
}

CScriptUploadEngine::~CScriptUploadEngine()
{

}

void CScriptUploadEngine::PrintCallback(const std::string& output)
{
    Log(ErrorInfo::mtInformation, output);
}

int CScriptUploadEngine::doProcessTask(std::shared_ptr<UploadTask> task, UploadParams& params) {
    if (task->type() == UploadTask::TypeAuth) {
        return processAuthTask(task);
    } if (task->type() == UploadTask::TypeTest) {
        return processTestTask(task);
    } if (task->type() == UploadTask::TypeFolder) {
        return processFolderTask(task);
    } else {
        return doUpload(task, params);
    }
}


int CScriptUploadEngine::processAuthTask(std::shared_ptr<UploadTask> task) {
    SetStatus(stAuthorization);
    auto authTask = std::dynamic_pointer_cast<AuthTask>(task);
    if (!authTask) {
        return 0;
    }
    switch (authTask->authActionType()) {
    case AuthActionType::Login:
        return doLogin();
    case AuthActionType::Logout:
        return doLogout();
    }
    return 0;
}

int CScriptUploadEngine::processTestTask(std::shared_ptr<UploadTask> task) {
    using namespace Sqrat;
    try {
        checkCallingThread();

        Function func(vm_.GetRootTable(), "TestConnection");
        if (func.IsNull()) {
            return 0;
        }

        auto tbl = func.Evaluate<Sqrat::Table>();

        int res = static_cast<int>(ScriptAPI::GetValue(tbl->GetValue<SQInteger>("status")));
        auto msg = ScriptAPI::GetValue(tbl->GetValue<Sqrat::string>("message"));
        task->uploadResult()->message = msg;

        return res;
    }
    catch (const Sqrat::Exception& e) {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::processTestTask\r\n" + e.Message()); 
    }
    catch (std::exception& e) {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::processTestTask\r\n" + std::string(e.what()));
    }
    FlushSquirrelOutput();
    return 0;
}

int CScriptUploadEngine::doUpload(std::shared_ptr<UploadTask> task, UploadParams& params)
{
    if (checkAuth() < 1) {
        return -1;
    }
    using namespace Sqrat;
    std::string FileName;

    currentTask_ = task;
    if (task->type() == UploadTask::TypeFile ) {
        auto* fileTask = dynamic_cast<FileUploadTask*>(task.get());
        if (fileTask) {
            FileName = fileTask->getFileName();
        }
    }

    UploadTask* topLevelTask = task->parentTask() ? task->parentTask() : task.get();
    auto* topLevelFileTask = dynamic_cast<FileUploadTask*>(topLevelTask);
    if (topLevelFileTask) {
        setCurrentTopLevelFileName(topLevelFileTask->getFileName());
    }
    CFolderItem parent;

    {
        std::string folderID;
        std::lock_guard<std::mutex> guard(serverSync_->folderMutex());
        CFolderItem& newFolder = m_ServersSettings->newFolder;
        ServerProfile& serverProfile = task->serverProfile();
        if (serverProfile.folderId() == CFolderItem::NewFolderMark) {
            serverProfile.setFolderId(newFolder.getId());
            serverProfile.setFolderTitle(newFolder.getTitle());

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

    if (!params.folderId.empty() && task->onFolderUsed_) {
        task->onFolderUsed_(task.get());
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
            ival = ScriptAPI::GetValue(func.Evaluate<int>(FileName.c_str(), &params));
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
                ival = !params.DirectUrl.empty();
            }
        }
    }
    catch (NetworkClient::AbortedException & ) {
        throw;
    }
    catch (ServerSyncException& e) {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::uploadFile\r\n" + std::string(e.what()));
        ival = -1; // fatal error
    }
	catch (const Sqrat::Exception& e) {
    	if (!strcmp(e.what(), "unauthorized_exception")) {
            ival = -2;
    	} else {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::uploadFile\r\n" + std::string(e.what()));
    	}
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
    if (onNeedStop_) {
        shouldStop = onNeedStop_();  // delegate call
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
	if (functionExists("Authenticate")) {
        newAuthMode = true;
	}
    hasRefreshTokenFunc_ = functionExists("RefreshToken");
    return true;
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
        auto count = arr->GetSize();
        for (auto i = 0; i < count; i++)
        {
            std::string title = arr->GetSlot(i).Cast<std::string>();
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
    if (newAuthMode) {
        try {
            serverSync_->beginAuth();
        } catch (const ServerSyncException&) {
            return 0;
        }
    }
    
    defer<void> d([&] { // Run at function exit
        if (newAuthMode) {
            serverSync_->endAuth();
        }
    });

    if (serverSync_->isAuthPerformed()) {
        return 1;
    }
    try
    {
        checkCallingThread();

        Function func(vm_.GetRootTable(), newAuthMode ? "Authenticate" : "DoLogin");
        if (func.IsNull()) {
            return 0;
        }
        int res = ScriptAPI::GetValue(func.Evaluate<int>());
        serverSync_->setAuthPerformed(res == 1);
        /*if ( Error::Occurred(vm_.GetVM() ) ) {
            Log(ErrorInfo::mtError, "CScriptUploadEngine::doLogin\r\n" + std::string(Error::Message(vm_.GetVM()))); 
            return 0;
        }*/
        
        return res;
    }
    catch (const std::exception& e)
    {
        serverSync_->setAuthPerformed(false);
        Log(ErrorInfo::mtError, "CScriptUploadEngine::doLogin\r\n" + std::string(e.what()));
    }
    FlushSquirrelOutput();
    return 0;
}

int CScriptUploadEngine::doLogout() {
    using namespace Sqrat;
    try
    {
        checkCallingThread();

        Function func(vm_.GetRootTable(), "DoLogout");
        if (func.IsNull()) {
            return 0;
        }
        int res = ScriptAPI::GetValue(func.Evaluate<int>());
        return res;
    }
    catch (std::exception& e)
    {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::doLogout\r\n" + std::string(e.what()));
    }
    FlushSquirrelOutput();
    return 0;
}

int CScriptUploadEngine::modifyFolder(CFolderItem& folder)
{
    if (checkAuth() < 1) {
        return 0;
    }
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
    if (checkAuth() < 1) {
        return 0;
    }
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

std::string CScriptUploadEngine::name() const
{
    return name_;
}

int CScriptUploadEngine::createFolder(const CFolderItem& parent, CFolderItem& folder)
{
    if (checkAuth() < 1) {
        return 0;
    }
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

void CScriptUploadEngine::setNetworkClient(INetworkClient* nm)
{
    CAdvancedUploadEngine::setNetworkClient(nm);
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
    return newAuthMode || functionExists("DoLogin");
}

bool CScriptUploadEngine::functionExists(const std::string& name){
    using namespace Sqrat;
    try {
        checkCallingThread();
        Function func(vm_.GetRootTable(), name.c_str());
        if (func.IsNull()) {
            return false;
        }
    }
    catch (std::exception& e) {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::supportsBeforehandAuthorization\r\n" + std::string(e.what()));
        return false;
    }
    FlushSquirrelOutput();
    return true;
}

void CScriptUploadEngine::stop()
{
    ScriptAPI::StopAssociatedServices(vm_.GetVM());
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
    if (m_UploadData) {
        ei.ServerName = m_UploadData->Name;
    }
    if (currentTask_) {
        ei.FileName = currentTask_->toString();
        UploadTask* task = currentTask_->parentTask() ? currentTask_->parentTask() : currentTask_.get();
        auto* fileTask = dynamic_cast<FileUploadTask*>(task);
        if (fileTask) {
            ei.TopLevelFileName = fileTask->getFileName();
        }
    }
    ei.ThreadId = std::this_thread::get_id();
    ei.Script = IuCoreUtils::ExtractFileName(fileName_);
    
    ErrorMessage(ei);
}

bool CScriptUploadEngine::isAuthenticated() {
    using namespace Sqrat;
    try
    {
        checkCallingThread();

        Function func(vm_.GetRootTable(), "IsAuthenticated");
        if (func.IsNull()) {
            return false;
        }
        int res = ScriptAPI::GetValue(func.Evaluate<int>());
        return res;
    }
    catch (std::exception& e)
    {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::isAuthenticated\r\n" + std::string(e.what()));
    }
    FlushSquirrelOutput();
    return false;
}

bool CScriptUploadEngine::supportsLogout() {
    using namespace Sqrat;
    try {
        checkCallingThread();
        Function func(vm_.GetRootTable(), "DoLogout");
        if (func.IsNull()) {
            return false;
        }
    }
    catch (std::exception& e) {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::supportsLogout\r\n" + std::string(e.what()));
        return false;
    }
    FlushSquirrelOutput();
    return true;
}

int CScriptUploadEngine::checkAuth() {
    using namespace Sqrat;
    int res = 1, res2 = 1;
	if (newAuthMode && m_UploadData->NeedAuthorization && m_ServersSettings->authData.DoAuth) {
		if(!serverSync_->isAuthPerformed()) {
            try {
                checkCallingThread();
                
                if (!isAuthenticated()) {
                	try {
                        res = doLogin();
                	} catch (const std::exception& e) {
                        Log(ErrorInfo::mtError, "CScriptUploadEngine::checkAuth\r\n" + std::string(e.what()));
                	}
                }
            	
            } catch (const std::exception& e) {
                Log(ErrorInfo::mtError, "CScriptUploadEngine::checkAuth\r\n" + std::string(e.what()));
            }  
		}
	}
	 
    if (hasRefreshTokenFunc_) {
        res2 = refreshToken();      
    }
    return static_cast<int>(res>=1 && res2>=1);
}

int CScriptUploadEngine::refreshToken()
{
    using namespace Sqrat;

    try
    {
        checkCallingThread();

        std::lock_guard<std::mutex> lk(serverSync_->refreshTokenMutex());
        Function func(vm_.GetRootTable(), "RefreshToken");
        if (func.IsNull()) {
            return 0;
        }
        int res = ScriptAPI::GetValue(func.Evaluate<int>());
        

        return res;
    }
    catch (std::exception& e)
    {
        Log(ErrorInfo::mtError, "CScriptUploadEngine::doLogin\r\n" + std::string(e.what()));
    }
    FlushSquirrelOutput();
    return 0;
}