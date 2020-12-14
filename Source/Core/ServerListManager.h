#ifndef IU_CORE_SERVERLISTMANAGER_H
#define IU_CORE_SERVERLISTMANAGER_H

#include <string>

#include "Core/Utils/CoreTypes.h"
#include "Core/Upload/UploadEngine.h"

class CUploadEngineList;

class ServerListManager
{
public:
    ServerListManager(const std::string &serversDirectory, CUploadEngineList* uel, ServerSettingsMap& serversSettings);
    ~ServerListManager()=default;
    std::string addFtpServer(const std::string &name, const std::string &serverName, const std::string &login, const std::string &password,
        const std::string &remoteDirectory, const std::string &downloadUrl);
    std::string addDirectoryAsServer(const std::string &name, const std::string &directory, const std::string &downloadUrl, bool convertUncPath);
protected:
    DISALLOW_COPY_AND_ASSIGN(ServerListManager);
    CUploadEngineList * uploadEngineList_;
    std::string serversDirectory_;
    ServerSettingsMap& serversSettings_;
};

#endif