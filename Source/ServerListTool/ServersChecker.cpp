#include "ServersChecker.h"

#include "Core/Upload/UploadSession.h"
#include "Core/Settings.h"
#include "Core/AppParams.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/Upload/UrlShorteningTask.h"
#include "Core/Upload/UploadManager.h"
#include "ServersCheckerModel.h"
#include "Func/WinUtils.h"
#include "Core/Utils/CryptoUtils.h"
#include "ServerListTool/Helpers.h"
#include "Core/ServiceLocator.h"
#include "Core/TaskDispatcher.h"

namespace ServersListTool {

ServersChecker::ServersChecker(ServersCheckerModel* model, UploadManager* uploadManager, std::shared_ptr<INetworkClientFactory> networkClientFactory) :
    model_(model),
    uploadManager_(uploadManager),
    useAccounts_(true),
    checkImageServers_(true),
    checkFileServers_(true),
    checkURLShorteners_(true),
    networkClientFactory_(networkClientFactory)

{
    needStop_ = false;
    isRunning_ = false;
    using namespace std::placeholders;
    fileDownloader_ = std::make_unique<CFileDownloader>(networkClientFactory_, AppParams::instance()->tempDirectory());
    fileDownloader_->setOnFileFinishedCallback(std::bind(&ServersChecker::OnFileFinished, this, _1, _2, _3));
}

bool ServersChecker::start(const std::string& testFileName, const std::string& testUrl) {
    GetFileInfo(U2W(testFileName), &m_sourceFileInfo);
    srcFileHash_ = IuCoreUtils::CryptoUtils::CalcMD5HashFromFile(testFileName);
    uploadSession_ = std::make_shared<UploadSession>();
    uploadSession_->addSessionFinishedCallback(UploadSession::SessionFinishedCallback(this, &ServersChecker::onSessionFinished));
    int taskCount = 0;
    for (size_t i = 0; i < model_->getCount(); i++) {
        ServerData* item = model_->getDataByIndex(i);
        if (item->skip) {
            continue;
        }

        //uploader.ShouldStop = &m_NeedStop;
        CUploadEngineData *ue = item->ued;
        if (!(ue->hasType(CUploadEngineData::TypeImageServer) && checkImageServers_) &&
            !(ue->hasType(CUploadEngineData::TypeFileServer) && checkFileServers_) &&
            !(ue->hasType(CUploadEngineData::TypeUrlShorteningServer) && checkURLShorteners_)) {
            continue;
        }

        ServerSettingsStruct  ss;
        auto serverIt = Settings.ServersSettings.find(ue->Name);
        if (serverIt != Settings.ServersSettings.end()) {
            std::map <std::string, ServerSettingsStruct>::const_iterator it = serverIt->second.begin();

            if (!useAccounts_) {
                if (it != serverIt->second.end()) {
                    ss = it->second;
                }
            } else {
                if (it != serverIt->second.end()) {
                    if (it->first.empty()) {
                        ++it;
                        if (it != serverIt->second.end()) {
                            ss = it->second;
                        }
                    } else {
                        ss = it->second;
                    }
                }
            }
        }

        if ((ue->NeedAuthorization == CUploadEngineData::naObligatory || (onlyAccs_ && ue->NeedAuthorization)) && ss.authData.Login.empty()) {
            item->setStatusText("No account is set");
            model_->notifyRowChanged(i);
            continue;
        }
        if (onlyAccs_ && !ue->NeedAuthorization) {
            item->setStatusText("skipped");
            model_->notifyRowChanged(i);
            continue;
        }
        ServerProfile serverProfile(ue->Name);
        serverProfile.setShortenLinks(false);
        serverProfile.setProfileName(ss.authData.Login);

        std::shared_ptr<UploadTask>  task;
        if (ue->hasType(CUploadEngineData::TypeImageServer) || ue->hasType(CUploadEngineData::TypeFileServer)) {
            task = std::make_shared<FileUploadTask>(testFileName, IuCoreUtils::ExtractFileName(testFileName));

        } else if (ue->hasType(CUploadEngineData::TypeUrlShorteningServer)) {
            task = std::make_shared<UrlShorteningTask>(testUrl);
        }

        if (task) {
            task->setServerProfile(serverProfile);
            task->OnStatusChanged.bind(this, &ServersChecker::onTaskStatusChanged);
            task->addTaskFinishedCallback(UploadTask::TaskFinishedCallback(this, &ServersChecker::onTaskFinished));
            UploadTaskUserData* userData = new UploadTaskUserData();
            userData->rowIndex = i;
            task->setUserData(userData);
            uploadTaskUserDatas_.push_back(std::unique_ptr<UploadTaskUserData>(userData));
            uploadSession_->addTask(task);
            taskCount++;
        }
    }
    if (taskCount) {
        uploadManager_->addSession(uploadSession_);
        uploadManager_->start();
    } else {
        processFinished();
    }
    return true;
}

void ServersChecker::stop() {
    needStop_ = true;
    if (fileDownloader_ && fileDownloader_->isRunning()) {
        fileDownloader_->stop();
    }
    if (uploadSession_) {
        uploadSession_->stop();
    }

}

bool ServersChecker::isRunning() const {
    return isRunning_ || (fileDownloader_ && fileDownloader_->isRunning());
}

void ServersChecker::setUseAccounts(bool value) {
    useAccounts_ = value;
}

void ServersChecker::setOnlyAccs(bool value) {
    onlyAccs_ = value;
}

void ServersChecker::setCheckImageServers(bool value) {
    checkImageServers_ = value;
}

void ServersChecker::setCheckFileServers(bool value) {
    checkFileServers_ = value;
}

void ServersChecker::setCheckUrlShorteners(bool value) {
    checkURLShorteners_ = value;
}

void ServersChecker::setOnFinishedCallback(std::function<void()> callback) {
    onFinishedCallback_ = callback;
}


bool ServersChecker::OnFileFinished(bool ok, int /*statusCode*/, CFileDownloader::DownloadFileListItem it)
{
    int serverId = reinterpret_cast<int>(it.id) / 10;
    int fileId = reinterpret_cast<int>(it.id) % 10;

    ServerData& serverData = *model_->getDataByIndex(serverId);
    serverData.filesChecked++;
    serverData.fileToCheck--;
    CString fileName = Utf8ToWstring(it.fileName).c_str();

    if (!ok) {
        serverData.stars[fileId] = 0;
        serverData.setLinkInfo(fileId, "Cannot download file");
    }
    if (IuCoreUtils::CryptoUtils::CalcMD5HashFromFile(W2U(fileName)) == srcFileHash_) {
        if (fileId == 0)
            serverData.stars[fileId] = 5;
        else serverData.stars[fileId] = 4;
        serverData.setLinkInfo(fileId, "Identical file");
    } else {
        Helpers::MyFileInfo mfi;

        CString report = GetFileInfo(fileName, &mfi);

        CString mimeType = Utf8ToWCstring(IuCoreUtils::GetFileMimeType(WCstringToUtf8((fileName))));
        if (fileId < 2) // is thumb or image
        {
            if (mimeType.Find(_T("image/")) >= 0) {
                if (fileId == 0 && (mfi.width != m_sourceFileInfo.width || mfi.height != m_sourceFileInfo.height))
                    serverData.stars[fileId] = 0;
                else
                    serverData.stars[fileId] = fileId == 0 ? 4 : 5;
            } else serverData.stars[fileId] = 0;
        } else serverData.stars[fileId] = 5;

        serverData.setLinkInfo(fileId, W2U(report));
    }
    if (serverData.fileToCheck == 0) {
        serverData.finished = true;
    }
    MarkServer(serverId);
    return 0;
}

void ServersChecker::checkShortUrl(UploadTask* task) {
    auto client = networkClientFactory_->create();
    UrlShorteningTask* urlTask = dynamic_cast<UrlShorteningTask*>(task);
    if (!urlTask) {
        return;
    }
    UploadTaskUserData* userData = reinterpret_cast<UploadTaskUserData*>(task->userData());
    ServerData& data = *model_->getDataByIndex(userData->rowIndex);

    client->setCurlOptionInt(CURLOPT_FOLLOWLOCATION, 0);

    bool ok = false;
    std::string targetUrl = task->uploadResult()->directUrl;
    int i = 0; //counter for limiting max redirects
    if (!targetUrl.empty()) {
        int responseCode = 0;
        do {
            client->setCurlOptionInt(CURLOPT_FOLLOWLOCATION, 0);
            client->doGet(targetUrl);
            responseCode = client->responseCode();
            targetUrl = client->responseHeaderByName("Location");
            i++;
        } while (i < 6 && !targetUrl.empty() && (responseCode == 302 || responseCode == 301) && targetUrl != urlTask->getUrl());

        if (!targetUrl.empty() && targetUrl == urlTask->getUrl()) {
            data.setStrMark("Good link");
            //m_ListView.SetItemText(userData->rowIndex, 3, _T("Good link"));
            ok = true;
        }
    }

    data.filesChecked++;
    data.stars[0] = ok ? 5 : 0;
    data.finished = true;
    MarkServer(userData->rowIndex);
}

void ServersChecker::onTaskFinished(UploadTask* task, bool ok) {
    CUploadEngineData* ue = task->serverProfile().uploadEngineData();
    UploadTaskUserData* userData = reinterpret_cast<UploadTaskUserData*>(task->userData());
    int i = userData->rowIndex;
    ServerData& data = *model_->getDataByIndex(i);
    if (task->status() == UploadTask::StatusStopped) {
        data.setStatusText(std::string());
        return;
    }
    if (ok) {
        DWORD endTime = GetTickCount() - userData->startTime;
        UploadResult* result = task->uploadResult();
        data.timeElapsed = endTime;

        if (task->type() == UploadTask::TypeFile) {
            std::string imgUrl = result->getDirectUrl();
            std::string thumbUrl = result->getThumbUrl();
            std::string viewUrl = result->getDownloadUrl();
            int nFilesToCheck = 0;
            if (!imgUrl.empty()) {
                fileDownloader_->addFile(result->getDirectUrl(), reinterpret_cast<void*>(i * 10));
                nFilesToCheck++;
                data.setDirectUrl(imgUrl);

            } else {
                if (!ue->ImageUrlTemplate.empty()) {
                    /*if (!ue->UsingPlugin)
                    m_CheckedServersMap[i].filesChecked++;*/
                    data.setDirectUrlInfo("<empty>");
                }
            }

            if (!thumbUrl.empty()) {
                nFilesToCheck++;
                fileDownloader_->addFile(result->getThumbUrl(), reinterpret_cast<void*>(i * 10 + 1));

                data.setThumbUrl(thumbUrl);
            } else {
                if (!ue->ThumbUrlTemplate.empty()) {
                    /*if (!ue->UsingPlugin)
                    m_CheckedServersMap[i].filesChecked++;*/
                    data.setThumbUrlInfo("<empty>");
                }
            }

            if (!viewUrl.empty()) {
                nFilesToCheck++;
                fileDownloader_->addFile(result->getDownloadUrl(), reinterpret_cast<void*>(i * 10 + 2));
                data.setViewUrl(viewUrl);
            } else {

                if (!ue->DownloadUrlTemplate.empty()) {
                    /*if (!ue->UsingPlugin)
                    m_CheckedServersMap[i].filesChecked++;*/
                    data.setViewUrlInfo("<empty>");
                }

            }
            data.fileToCheck = nFilesToCheck;
            fileDownloader_->start();
            if (nFilesToCheck)
                data.setStatusText( "Checking links");
            else {
                data.finished = true;
            }
        } else if (task->type() == UploadTask::TypeUrl) {
            std::string shortURL = result->getDirectUrl();
            if (!shortURL.empty()) {
                data.setDirectUrl(shortURL);
                data.setStatusText( "Checking short link");
                checkShortUrl(task);
            } else {
                data.finished = true;
                data.setDirectUrlInfo("<empty>");
            }
        }


    } else {
        data.finished = true;
    }
    MarkServer(i);
}

void ServersChecker::onSessionFinished(UploadSession* session) {
    processFinished();
    LOG(INFO) << "Uploader has finished";
}

void ServersChecker::onTaskStatusChanged(UploadTask* task) {
    CUploadEngineData* ue = task->serverProfile().uploadEngineData();
    UploadTaskUserData* userData = reinterpret_cast<UploadTaskUserData*>(task->userData());
    int i = userData->rowIndex;
    if (task->status() == UploadTask::StatusRunning) {
        userData->startTime = GetTickCount();
        ServerData* data = model_->getDataByIndex(i);
        data->setStatusText(task->type() == UploadTask::TypeUrl ? "Shortening link..." : "Uploading file...");
        model_->notifyRowChanged(i);
    } else if (task->status() == UploadTask::StatusFinished) {

    } else if (task->status() == UploadTask::StatusFailure) {

    }
}

void ServersChecker::MarkServer(int id)
{
    ServerData& serverData = *model_->getDataByIndex(id);
    if (serverData.finished) {
        int sum = serverData.stars[0] + serverData.stars[1] + serverData.stars[2];
        int mark = 0;
        int count = serverData.filesChecked;
        if (count) mark = sum / count;

        CString timeLabel;
        int endTime = serverData.timeElapsed;
        timeLabel.Format(_T("%02d:%02d"), (int)(endTime / 60000), (int)(endTime / 1000 % 60));
        serverData.setTimeStr(W2U(timeLabel));

        CString strMark;
        if (mark == 5) {
            serverData.setStrMark("EXCELLENT");
            serverData.color = RGB(0, 255, 50);

        } else if (mark >= 4) {
            serverData.setStrMark("OK");
            serverData.color = RGB(145, 213, 0);
            //m_ListView.SetItemText(id*2,2,CString());
        } else {
            serverData.setStrMark("FAILED");
            serverData.color = RGB(198, 0, 0);
            //m_CheckedServersMap[id].failed = true;
        }
    }

    ServiceLocator::instance()->taskDispatcher()->runInGuiThread([&] {
        model_->notifyRowChanged(id);
    });


}

void ServersChecker::processFinished() {
    isRunning_ = false;
    if (onFinishedCallback_) {
        onFinishedCallback_();
    }
}


}