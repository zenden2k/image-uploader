#include "CurlShare.h"
#include "Core/Logging.h"

CurlShare::CurlShare()
{
    share_ = curl_share_init();
    CURLSHcode shareError;
    curl_share_setopt(share_, CURLSHOPT_USERDATA, this);
    
    shareError = curl_share_setopt(share_, CURLSHOPT_LOCKFUNC, lockData);
    if (shareError){
        LOG(ERROR) << "share set opt wrong" ;
        return;
    }
    shareError = curl_share_setopt(share_, CURLSHOPT_UNLOCKFUNC, unlockData);
    if (shareError){
        LOG(ERROR) << "share set opt wrong";
        return;
    }
    shareError = curl_share_setopt(share_, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    if (shareError){
        LOG(ERROR) << "share set opt wrong";
        return;
    }
    shareError = curl_share_setopt(share_, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
    if (shareError){
        LOG(ERROR) << "share set opt wrong";
        return;
    }
    shareError = curl_share_setopt(share_, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
    if (shareError){
        LOG(ERROR) << "share set opt wrong";
        return;
    }
}

CurlShare::~CurlShare()
{
    curl_share_cleanup(share_);
}

CURLSH* CurlShare::getHandle() const
{
    return share_;
}

void CurlShare::lockData(CURL *handle, curl_lock_data data, curl_lock_access, void *useptr){
    CurlShare* pthis = reinterpret_cast<CurlShare*>(useptr);
    pthis->mutexes_[data].lock();
}
/* unlock callback */
void CurlShare::unlockData(CURL *handle, curl_lock_data data, void *useptr){
    CurlShare* pthis = reinterpret_cast<CurlShare*>(useptr);
    pthis->mutexes_[data].unlock();
}