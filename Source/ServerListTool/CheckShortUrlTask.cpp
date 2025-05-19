#include "CheckShortUrlTask.h"

namespace ServersListTool {

CheckShortUrlTask::CheckShortUrlTask(std::shared_ptr<INetworkClientFactory> factory, std::string url, std::string shortUrl)
    : isCanceled_(false)
    , isInProgress_(false)
    , factory_(std::move(factory))
    , url_(std::move(url))
    , shortUrl_(std::move(shortUrl))
{
}

void CheckShortUrlTask::run()
{
    if (isCanceled_) {
        onTaskFinished(this, false);
        return;
    }
    isInProgress_ = true;
    auto client = factory_->create();

    client->setCurlOptionInt(CURLOPT_FOLLOWLOCATION, 0);

    bool ok = false;
    std::string targetUrl = url_;
    int i = 0; //counter for limiting max redirects
    if (!targetUrl.empty()) {
        int responseCode = 0;

        do {
            if (isCanceled_) {
                break;
            }
            client->setCurlOptionInt(CURLOPT_FOLLOWLOCATION, 0);
            client->doGet(targetUrl);

            responseCode = client->responseCode();
            if (responseCode == 302 || responseCode == 301) {
                targetUrl = client->getCurlInfoString(CURLINFO_REDIRECT_URL);
            }

            i++;
        } while (i < 6 && !targetUrl.empty() && (responseCode == 302 || responseCode == 301) && targetUrl != shortUrl_);

        if (!targetUrl.empty() && targetUrl == shortUrl_) {
            ok = true;
        }
    }

    isInProgress_ = false;
    onTaskFinished(this, ok);
}

void CheckShortUrlTask::cancel()
{
    isCanceled_ = true;
}

bool CheckShortUrlTask::isCanceled()
{
    return isCanceled_;
}

bool CheckShortUrlTask::isInProgress()
{
    return isInProgress_;
}

}
