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

#ifndef IU_DISABLE_MEGANZ

#include "MegaNzUploadEngine.h"

#include <chrono>
#include <thread>

#include <megaapi.h>
#include "Core/Upload/ServerSync.h"
#include "Uploader.h"
#include "Core/Settings/BasicSettings.h"
#include "FileUploadTask.h"
#include "Core/CommonDefs.h"
#include "Core/i18n/Translator.h"
#ifdef _WIN32
#include "Func/IuCommonFunctions.h"
#include "3rdpart/GdiplusH.h"
#include "Core/Images/Utils.h"
#endif

#define APP_KEY "0dxDFKqD"
#define USER_AGENT "Zenden2k Image Uploader"

using namespace mega;

#ifdef _WIN32

class MyGfxProcessor : public mega::MegaGfxProcessor {
public:
    MyGfxProcessor();
    virtual bool readBitmap(const std::string& path) override;
    virtual int getWidth() override;
    virtual int getHeight() override;
    virtual int getBitmapDataSize(int width, int height, int px, int py, int rw, int rh) override;
    virtual bool getBitmapData(char *bitmapData, size_t size) override;
    virtual void freeBitmap() override;

    virtual ~MyGfxProcessor();
protected:
    std::unique_ptr<Gdiplus::Bitmap> bitmap_; 
    HGLOBAL hGlobal_;
    CComPtr<IStream> istream_;
};

MyGfxProcessor::MyGfxProcessor() {
    hGlobal_ = nullptr;
}
bool MyGfxProcessor::readBitmap(const std::string& path) {
    // path parameter contains utf-16 string
    CString widePath(reinterpret_cast<const wchar_t*>(path.data()), path.size() / 2);

    if (!IuCommonFunctions::IsImage(widePath)) {
        return false;
    }
    bitmap_ = ImageUtils::LoadImageFromFileExtended(widePath);
    if (!bitmap_) {
        return false;
    }
    return true;
}

int MyGfxProcessor::getWidth() {
    if (bitmap_) {
        return bitmap_->GetWidth();
    }
    return 0;
}

int MyGfxProcessor::getHeight() {
    if (bitmap_) {
        return bitmap_->GetHeight();
    }
    return 0;
}

int MyGfxProcessor::getBitmapDataSize(int width, int height, int px, int py, int rw, int rh) {
    using namespace Gdiplus;
    if (!bitmap_) {
        return 0;
    }
    std::unique_ptr<Bitmap> res = std::make_unique<Bitmap>(width, height);
    Graphics gr(res.get());
    gr.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    gr.DrawImage(bitmap_.get(), 0, 0, width, height);

    std::unique_ptr<Bitmap> resultBmp;
    if (px == 0 && py == 0 && rw == width && rh == height) {
        resultBmp = std::move(res);
    } else {
        std::unique_ptr<Bitmap> buffer = std::make_unique<Bitmap>(rw, rh);
        Graphics gr2(buffer.get());
        gr2.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        gr2.DrawImage(res.get(), 0, 0, px, py, rw, rh, UnitPixel);
        resultBmp = std::move(buffer);
    }

    if (!resultBmp) {
        return 0;
    }
    
    HRESULT hr = CreateStreamOnHGlobal(nullptr, TRUE, &istream_);
    if (FAILED(hr)) {
        return 0;
    }

    // Saving image in JPEG format to memory stream
    if (ImageUtils::SaveImageToFile(resultBmp.get(), CString(), istream_, ImageUtils::sifJPEG, 85)) {
        hr = GetHGlobalFromStream(istream_, &hGlobal_);
        if (FAILED(hr)) {
            return 0;
        }
        return GlobalSize(hGlobal_);
    }
    return 0;
}

bool MyGfxProcessor::getBitmapData(char* bitmapData, size_t size) {
    if (!hGlobal_) {
        return false;
    }
    LPVOID pimage = GlobalLock(hGlobal_);
    if (pimage == nullptr) {
        return false;
    }
    memcpy(bitmapData, pimage, size);
    GlobalUnlock(hGlobal_);
    istream_.Release();
    return true;
}

void MyGfxProcessor::freeBitmap() {
    hGlobal_ = nullptr;
    bitmap_.reset();
}

MyGfxProcessor::~MyGfxProcessor() {
}

#endif // End of #ifdef _WIN32

class MyListener : public mega::MegaListener {
public:
    explicit MyListener(CMegaNzUploadEngine* engine) {
        engine_ = engine;
    }
    ~MyListener() {
        
    }
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;
    //Currently, this callback is only valid for the request fetchNodes()
    virtual void onRequestUpdate(mega::MegaApi*api, mega::MegaRequest *request) override;
    virtual void onRequestTemporaryError(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError* error) override;
    virtual void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* error) override;
    virtual void onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer) override;
    virtual void onTransferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError* error) override;
    virtual void onUsersUpdate(mega::MegaApi* api, mega::MegaUserList *users) override;
    virtual void onNodesUpdate(mega::MegaApi* api, mega::MegaNodeList *nodes) override;
private:
    CMegaNzUploadEngine* engine_;
};
CMegaNzUploadEngine::CMegaNzUploadEngine(ServerSync* serverSync, ServerSettingsStruct* settings, ErrorMessageCallback errorCallback) :
    CAdvancedUploadEngine(serverSync, settings, errorCallback)
{
    setServerSettings(settings);
    folderList_ = nullptr;
#ifdef _WIN32
    proc_ = std::make_unique<MyGfxProcessor>();
    megaApi_ = std::make_unique<MegaApi>(APP_KEY, proc_.get(), static_cast<const char *>(nullptr), USER_AGENT);
#else 
    megaApi_.reset(new MegaApi(APP_KEY, (const char *)NULL, USER_AGENT));
#endif
    megaApi_->setLogLevel(MegaApi::LOG_LEVEL_INFO);
    listener_.reset(new MyListener(this));
    megaApi_->addListener(listener_.get());
    proxy_.reset(new MegaProxy());
    BasicSettings& Settings = *ServiceLocator::instance()->basicSettings();

    if (Settings.ConnectionSettings.UseProxy == ConnectionSettingsStruct::kUserProxy) {
        if (Settings.ConnectionSettings.ProxyType != 0) {
            Log(ErrorInfo::mtError, "This proxy type is not supported by Mega.Nz engine.");
        }
        else {
            proxy_->setProxyType(MegaProxy::PROXY_CUSTOM);
            proxy_->setProxyURL((std::string("http://") + Settings.ConnectionSettings.ServerAddress + ":" + IuCoreUtils::toString(Settings.ConnectionSettings.ProxyPort) + "/").c_str());
            if (Settings.ConnectionSettings.NeedsAuth) {
                proxy_->setCredentials(Settings.ConnectionSettings.ProxyUser.c_str(), std::string(Settings.ConnectionSettings.ProxyPassword).c_str());
            }
        }
    }
    else if (Settings.ConnectionSettings.UseProxy == ConnectionSettingsStruct::kSystemProxy) {
        proxy_.reset(megaApi_->getAutoProxySettings());
    }
    else {
        proxy_->setProxyType(MegaProxy::PROXY_NONE);
    }
    if (proxy_) {
        megaApi_->setProxySettings(proxy_.get());
    }
}

CMegaNzUploadEngine::~CMegaNzUploadEngine()
{
    megaApi_->removeListener(listener_.get());
}

int CMegaNzUploadEngine::getFolderList(CFolderList& FolderList) {
    if (!loginFinished_) {
        if (!doLogin()) {
            return 0;
        }
    }
    folderList_ = &FolderList;
    if (ensureNodesFetched()) {
        FolderList.AddFolder("/", "", std::string("/"), "", 0);
        MegaNode *root = megaApi_->getRootNode();
        MegaNodeList *list = megaApi_->getChildren(root);

        for (int i = 0; i < list->size(); i++) {
            MegaNode *node = list->get(i);
            if (node->isFolder()) {
                FolderList.AddFolder(node->getName(), "", std::string("/")+node->getName(), "/", 0);
            }
        }
        delete list;
        delete root;
    }
    
    return fetchNodesSuccess_ ? 1 : 0;;
}

int CMegaNzUploadEngine::createFolder(const CFolderItem& parent, CFolderItem& folder) {
    if (!loginFinished_) {
        if (!doLogin()) {
            return 0;
        }
    }

    if (needStop()) {
        return 0;
    }

    createFolderSuccess_ = false;
    createFolderFinished_ = false;
    if (ensureNodesFetched()) {
        std::unique_ptr<MegaNode> parentNode;
        if (parent.getId().empty()) {
            parentNode.reset(megaApi_->getRootNode());
        } else {
            parentNode.reset(megaApi_->getNodeByPath(parent.getId().c_str()));
        }
       
        if (!parentNode || !parentNode->isFolder()) {
            Log(ErrorInfo::mtError, "Unable to find parent folder");
            return 0;
        }
        megaApi_->createFolder(folder.getTitle().c_str(), parentNode.get());
        while (!createFolderFinished_ && !needStop()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return createFolderSuccess_ ? 1 : 0;
    }
    return 0;
}

int CMegaNzUploadEngine::modifyFolder(CFolderItem& folder) {
    if (!loginFinished_) {
        if (!doLogin()) {
            return 0;
        }
    }

    if (needStop()) {
        return 0;
    }

    renameFolderFinished_ = false;
    renameFolderSuccess_ = false;
    if (ensureNodesFetched()) {
        std::unique_ptr<MegaNode> node(megaApi_->getNodeByPath(folder.getId().c_str()));
        
        if (!node || !node->isFolder()) {
            Log(ErrorInfo::mtError, "Unable to find folder '"+folder.getId()+"'");
            return 0;
        }
        megaApi_->renameNode(node.get(), folder.getTitle().c_str());
        while (!renameFolderFinished_ && !needStop()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return renameFolderSuccess_ ? 1 : 0;
    }

    return 0;
}

int CMegaNzUploadEngine::getAccessTypeList(std::vector<std::string>& list) {
    list.push_back("Default");
    return 1;
}

int CMegaNzUploadEngine::getServerParamList(std::map<std::string, std::string>& list) {
    return 0;
}

int CMegaNzUploadEngine::doLogin() {
    loginFinished_ = false;
    loginSuccess_ = false;
    if (currentTask_) {
        currentTask_->setStatusText(tr("Logging in..."));
    }
    megaApi_->login(m_ServersSettings->authData.Login.c_str(), m_ServersSettings->authData.Password.c_str());
    while (!loginFinished_ && !needStop()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return loginSuccess_ ? 1 : 0;
}

int CMegaNzUploadEngine::doUpload(std::shared_ptr<UploadTask> task, UploadParams& params)
{
    if (task->type() != UploadTask::TypeFile) {
        return 0;
    }
    currentTask_ = task;

    if (!loginFinished_) {
        if (!doLogin()) {
            return 0;
        }
    }

    if (ensureNodesFetched()) {
        publicLink_.clear();
        fileTask_ = std::dynamic_pointer_cast<FileUploadTask>(task); 
        std::unique_ptr<MegaNode> root;
        std::string folderId = fileTask_->serverProfile().folderId();
        if (folderId.empty()) {
            root.reset(megaApi_->getRootNode());
        } else {
            root.reset(megaApi_->getNodeByPath(folderId.c_str()));
            if (root == nullptr) {
                Log(ErrorInfo::mtError, "Folder '" + folderId + "' not found");
                return 0;
            }
        }
       
        std::string origFileName = IuCoreUtils::ExtractFileName(fileTask_->getFileName());
        std::string newFileName = origFileName;
        std::unique_ptr<MegaNode> node(megaApi_->getChildNode(root.get(), origFileName.c_str()));
        int i = 2;
        while (node) {
            newFileName = IuCoreUtils::incrementFileName(origFileName, i++);
            node.reset(megaApi_->getChildNode(root.get(), newFileName.c_str()));
        } 

        uploadFinished_ = false;
        uploadSuccess_ = false;

        megaApi_->startUpload(fileTask_->getFileName().c_str(), root.get(),newFileName.c_str());
        while (!uploadFinished_ && !needStop()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (!uploadSuccess_ || needStop()) {
            return 0;
        }
        while (!exportFinished_ && !needStop()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (exportSuccess_) {
            params.setViewUrl(publicLink_);
            return 1;
        }

    }
    return 0;
}

bool CMegaNzUploadEngine::supportsSettings()
{
    return false;
}

bool CMegaNzUploadEngine::supportsBeforehandAuthorization()
{
    return true;
}

bool CMegaNzUploadEngine::ensureNodesFetched() {
    std::unique_ptr<MegaNode> root(megaApi_->getRootNode());
    if (!root) {
        fetchNodesFinished_ = false;
        fetchNodesSuccess_ = false;
        if (currentTask_) {
            currentTask_->setStatusText(tr("Fetching filesystem..."));
        }
        megaApi_->fetchNodes();
        while (!fetchNodesFinished_ && !needStop()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    root.reset(megaApi_->getRootNode());
    return root != nullptr;
}

void MyListener::onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e)
{
    switch (request->getType()) {
    case MegaRequest::TYPE_LOGIN:
    {
        engine_->loginSuccess_ = e->getErrorCode() == MegaError::API_OK;
        if (!engine_->loginSuccess_) {
            engine_->Log(ErrorInfo::mtError, "Login failed: " + std::string(e->getErrorString()));
        }
        engine_->loginFinished_ = true;
        break;
    }
    case MegaRequest::TYPE_FETCH_NODES:
    {
        if (e->getErrorCode() == MegaError::API_OK) {
            engine_->fetchNodesSuccess_ = true;
        } else {
            engine_->Log(ErrorInfo::mtError, "Unable to fetch remote filesystem: " + std::string(e->getErrorString()));
        }
        engine_->fetchNodesFinished_ = true;

        break;
    }
    case MegaRequest::TYPE_EXPORT:
    {
        if (e->getErrorCode() == MegaError::API_OK) {
            MegaHandle handle = request->getNodeHandle();
            std::unique_ptr<MegaNode> node(api->getNodeByHandle(handle));
            char* link = node->getPublicLink();
            if (link) {
                engine_->publicLink_ = link;
                //std::cout << "***** Public link " << link;
                engine_->exportSuccess_ = true;
                delete[] link;
            }
        }
        engine_->exportFinished_ = true;
        break;
    }
    case MegaRequest::TYPE_CREATE_FOLDER:
    {
        if (e->getErrorCode() == MegaError::API_OK) {
            engine_->createFolderSuccess_ = true;
            MegaHandle handle = request->getNodeHandle();
            std::unique_ptr<MegaNode> node(api->getNodeByHandle(handle));
            if (node) {
                engine_->createFolderSuccess_ = true;
            }
        } else {
            engine_->Log(ErrorInfo::mtError, "Unable to create folder: " + std::string(e->getErrorString()));
        }
        engine_->createFolderFinished_ = true;
    }
    break;
    case MegaRequest::TYPE_RENAME:
    {
        if (e->getErrorCode() == MegaError::API_OK) {
            engine_->renameFolderSuccess_ = true;
        } else {
            engine_->Log(ErrorInfo::mtError, "Unable to rename folder: " + std::string(e->getErrorString()));
        }
        engine_->renameFolderFinished_ = true;
    }
    break;
    default:
        break;
    }
}

//Currently, this callback is only valid for the request fetchNodes()
void MyListener::onRequestUpdate(MegaApi*api, MegaRequest *request)
{
    //std::cout << "***** Loading filesystem " << request->getTransferredBytes() << " / " << request->getTotalBytes() << std::endl;
}

void MyListener::onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* error)
{
    engine_->Log(ErrorInfo::mtWarning, "Mega.nz request error: " + std::string(error->getErrorString()));
}

void MyListener::onTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* error)
{
    if (error->getErrorCode()) {
        engine_->Log(ErrorInfo::mtError, "Transfer finished with error: " + std::string(error->getErrorString()));
    } else {
        MegaHandle handle = transfer->getNodeHandle();
        std::unique_ptr<MegaNode> node(api->getNodeByHandle(handle));
        engine_->exportFinished_ = false;
        engine_->exportSuccess_ = false;
        engine_->uploadSuccess_ = true;
        if (!engine_->needStop()) {
            api->exportNode(node.get());
        }
    }

    engine_->uploadFinished_ = true;
}

void MyListener::onTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{
    InfoProgress prInfo;
    prInfo.IsUploading = true;
    prInfo.Total = transfer->getTotalBytes();
    prInfo.Uploaded = transfer->getTransferredBytes();

    if (engine_->fileTask_) {
        CUploader* uploader = engine_->currentUploader();
        if (uploader) {
            uploader->SetStatus(stUploading);
            engine_->fileTask_->uploadProgress(prInfo);
            if (uploader->onProgress)
                uploader->onProgress(uploader, prInfo);
        }
    }

    if (engine_->needStop()) {
        api->cancelTransferByTag(transfer->getTag());
    }
}

void MyListener::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* error)
{
    //std::cout << "***** Temporary error in transfer: " << error->getErrorString() << std::endl;
}

void MyListener::onUsersUpdate(MegaApi* api, MegaUserList *users)
{
    if (users == NULL) {
        //Full account reload
        return;
    }
    //std::cout << "***** There are " << users->size() << " new or updated users in your account" << std::endl;
}

void MyListener::onNodesUpdate(MegaApi* api, MegaNodeList *nodes)
{
    if (nodes == NULL) {
        //Full account reload
        return;
    }

    //std::cout << "***** There are " << nodes->size() << " new or updated node/s in your account" << std::endl;
}
#endif