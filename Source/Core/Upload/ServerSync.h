#ifndef IU_CORE_UPLOAD_SERVERSYNC_H
#define IU_CORE_UPLOAD_SERVERSYNC_H

#pragma once


#include <mutex>
#include <memory>
#include "Core/ThreadSync.h"
#include "Core/Utils/CoreTypes.h"

class ServerSyncPrivate;
class ThreadSyncPrivate;

class ServerSyncException : public std::runtime_error {
public:
    ServerSyncException(const char* msg) : std::runtime_error(msg){}
};

/**
@brief ServerSync class contains server- (and account) specific data and tools for syncronization
threads which are uploading to the same server (and account)
*/
class ServerSync: public ThreadSync
{
    public:
        ServerSync();
        /**
        @throw ServerSyncException if authentication failed in another thread
        */
        bool beginAuth();
        bool endAuth();
        void setAuthPerformed(bool success);
        bool isAuthPerformed();
        
        /*! @cond PRIVATE */
        void resetAuthorization();
        void resetFailedAuthorization();
        void setConstVar(const std::string& name, const std::string& value);
        std::string getConstVar(const std::string& name);
        /* @endcond */
    private:
        Q_DECLARE_PRIVATE_PTR(ServerSync);
};

#endif
