#ifndef IU_CORE_UPLOAD_SERVERSYNC_H
#define IU_CORE_UPLOAD_SERVERSYNC_H

#pragma once


#include <mutex>
#include <memory>
#include "Core/ThreadSync.h"
#include "Core/Utils/CoreTypes.h"

class ServerSyncPrivate;
class ThreadSyncPrivate;
/**
    @brief ServerSync class contains server- (and account) specific data and tools for syncronization 
    threads which are uploading to the same server (and account)
*/
class ServerSync: public ThreadSync
{
    public:
        ServerSync();
        bool beginAuth();
        bool endAuth();
        void setAuthPerformed(bool success);
        bool isAuthPerformed();
    private:
        Q_DECLARE_PRIVATE_PTR(ServerSync);
};

#endif
