#ifndef IU_CORE_NETWORK_CURLSHARE_H
#define IU_CORE_NETWORK_CURLSHARE_H

#pragma once
#include <curl/curl.h>
#include <mutex>
#include "Core/Utils/CoreTypes.h"

class CurlShare {
public:
    CurlShare();
    ~CurlShare();
    CURLSH* getHandle() const;
private:
    DISALLOW_COPY_AND_ASSIGN(CurlShare);
    CURLSH* share_;
    std::mutex mutexes_[CURL_LOCK_DATA_LAST + 1];
    static void lockData(CURL *handle, curl_lock_data data, curl_lock_access access, void *useptr);
    static void unlockData(CURL *handle, curl_lock_data data, void *useptr);
};

#endif
