#ifndef IU_CORE_SERVERLISTMANAGER_H
#define IU_CORE_SERVERLISTMANAGER_H

#include <string>
#include <map>
#include <Core/Upload/UploadEngine.h>
class CUploadEngineList;


class ServerListManager
{
public:
	ServerListManager(const std::string &serversDirectory, CUploadEngineList* uel, std::map <std::string, ServerSettingsStruct>& serversSettings);
	~ServerListManager(void);
	bool addFtpServer(const std::string &name, const std::string &serverName, const std::string &login, const std::string &password,
		const std::string &remoteDirectory, const std::string &downloadUrl);
	bool addDirectoryAsServer(const std::string &name, const std::string &directory, const std::string &downloadUrl);
	std::string errorMessage() const;
	std::string createdServerName() const;
protected:
	CUploadEngineList * uploadEngineList_;
	std::string serversDirectory_;
	std::map <std::string, ServerSettingsStruct>& serversSettings_;
	std::string errorMessage_;
	std::string createdServerName_;

};

#endif