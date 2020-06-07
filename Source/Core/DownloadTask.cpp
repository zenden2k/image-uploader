#include "DownloadTask.h"

#include <boost/filesystem.hpp>

#include "Core/Utils/CoreUtils.h"
#include "Core/3rdpart/UriParser.h"
#include "Core/Utils/StringUtils.h"

DownloadTask::DownloadTask(std::shared_ptr<INetworkClientFactory> factory, const std::string& tempDirectory, const std::vector<DownloadItem>& files):
    factory_(std::move(factory)), files_(files), tempDirectory_(tempDirectory)
{
    isCanceled_ = false;
    isInProgress_ = false;
}

void DownloadTask::run() {
    if (isCanceled_) {
        onTaskFinished(this);
        return;
    }
    isInProgress_ = true;
    std::unique_ptr<INetworkClient> nm(factory_->create());
    using namespace std::placeholders;
    nm->setProgressCallback(std::bind(&DownloadTask::progressCallback, this, _1, _2, _3, _4, _5));

    for (auto& file : files_) {
        if (isCanceled_) {
            break;
        }
        uriparser::Uri uri(file.url);
        std::string path = uri.path();
        std::string fileName = nm->urlDecode(IuCoreUtils::ExtractFileName(path));

        if (fileName.length() > 30) {
            fileName = fileName.substr(0, 30);
        }
        file.displayName = fileName;
        std::string filePath;

        if (file.fileName.empty()) {
            boost::filesystem::path temp = boost::filesystem::unique_path();
            auto tempstr = temp.native();

#ifdef _WIN32
            filePath = IuCoreUtils::WstringToUtf8(tempstr);
#else
            filePath = tempstr;
#endif
        }
        else {
            filePath = file.fileName;
        }

        //if (createFileBeforeDownloading_) {
            // Creating file
        std::string fullFilePath = tempDirectory_ + filePath;
        FILE* f = IuCoreUtils::fopen_utf8(fullFilePath.c_str(), "wb");
        if (f) {
            fclose(f);
        }
        else {
            LOG(ERROR) << "Unable to create file:" << std::endl << filePath << std::endl
                << "Error: " << strerror(errno);
            return;
        }
        std::string url = file.url;
        url = IuStringUtils::Replace(url, " ", "%20");

        nm->setOutputFile(fullFilePath);
        if (!file.referer.empty()) {
            nm->setReferer(file.referer);
        }
        try {
            nm->doGet(url);
        }
        catch (INetworkClient::AbortedException&) {

        }

        bool success = nm->responseCode() >= 200 && nm->responseCode() <= 299;
        file.fileName = fullFilePath;
        onFileFinished(success, nm->responseCode(), file);
        //}
    }
    isInProgress_ = false;
    onTaskFinished(this);
}

void DownloadTask::cancel() {
    isCanceled_ = true;
    
}

bool DownloadTask::isCanceled() {
    return isCanceled_;
}

bool DownloadTask::isInProgress() {
    return isInProgress_;
}

int DownloadTask::progressCallback(INetworkClient* /* userData*/, double /*dltotal*/, double /*dlnow*/, double/* ultotal*/, double /*ulnow*/) {
    if (isCanceled_) {
        return -1;
    }

    return 0;
}