#ifndef IU_SERVERLISTTOOL_SERVERSCHECKER_H
#define IU_SERVERLISTTOOL_SERVERSCHECKER_H

#include <memory>
#include <vector>
#include "Core/Upload/UploadSession.h"
#include "Core/FileDownloader.h"
#include "Core/Network/INetworkClient.h"
#include "Helpers.h"

class CFileDownloader;

namespace ServersListTool {

class ServersCheckerModel;

struct UploadTaskUserData {
    int rowIndex;
    DWORD startTime;
    UploadTaskUserData() {
        rowIndex = 0;
        startTime = 0;
    }
};

class ServersChecker {
public:
    ServersChecker(ServersCheckerModel* model, UploadManager* uploadManager, std::shared_ptr<INetworkClientFactory> networkClientFactory);
    bool start(const std::string& testFileName, const std::string& testUrl = "");
    void stop();
    bool isRunning() const;
    void setUseAccounts(bool value);
    void setOnlyAccs(bool value);
    void setCheckImageServers(bool value);
    void setCheckFileServers(bool value);
    void setCheckUrlShorteners(bool value);
    void setOnFinishedCallback(std::function<void()> callback);

protected:
    void onTaskStatusChanged(UploadTask* task);
    void onTaskFinished(UploadTask* task, bool ok);
    void onSessionFinished(UploadSession* session);
    void OnFileFinished(bool ok, int statusCode, CFileDownloader::DownloadFileListItem it);
    void checkShortUrl(UploadTask* task);
    void MarkServer(int id);
    void processFinished();

    std::shared_ptr<UploadSession> uploadSession_;
    std::unique_ptr<CFileDownloader> fileDownloader_;
    std::atomic_bool needStop_, isRunning_;
    ServersCheckerModel* model_;
    UploadManager* uploadManager_;
    bool useAccounts_, onlyAccs_, checkImageServers_, checkFileServers_, checkURLShorteners_;
    std::vector<std::unique_ptr<UploadTaskUserData>> uploadTaskUserDatas_;
    std::function<void()> onFinishedCallback_;
    Helpers::MyFileInfo m_sourceFileInfo;
    std::string srcFileHash_;
    std::shared_ptr<INetworkClientFactory> networkClientFactory_;
};
}
#endif