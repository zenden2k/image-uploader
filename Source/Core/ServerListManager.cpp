
#include "ServerListManager.h"
#include <Core/Utils/SimpleXml.h>
#include "UploadEngineList.h"

ServerListManager::ServerListManager(const std::string &serversDirectory, CUploadEngineList* uel, std::map <std::string, ServerSettingsStruct>& serversSettings): 
	serversSettings_(serversSettings)
{
	uploadEngineList_ = uel;
	serversDirectory_ = serversDirectory;
	
}

ServerListManager::~ServerListManager(void)
{
}

bool ServerListManager::addFtpServer(const std::string &name, const std::string &serverName, const std::string &login, const std::string &password, const std::string &remoteDirectory, const std::string &downloadUrl)
{
	ZSimpleXml xml;
	ZSimpleXmlNode root = xml.getRoot("Servers");
	std::string newName = name + " (ftp)";

	if ( uploadEngineList_->byName(newName) ) {
		errorMessage_ = "Server with such name already exists.";
		return false;
	}

	ZSimpleXmlNode serverNode = root.GetChild("Server");
	serverNode.SetAttribute("Name", newName);
	serverNode.SetAttribute("Plugin", "ftp");
	serverNode.SetAttribute("FileHost", 1);
	serverNode.SetAttribute("Authorize", 1);

	ZSimpleXmlNode resultNode = serverNode.GetChild("Result");
	resultNode.SetAttribute("ImageUrlTemplate", "stub");
	resultNode.SetAttribute("ThumbUrlTemplate", "stub");
	resultNode.SetAttribute("DownloadUrlTemplate", "stub");

	std::string outFile = serversDirectory_ + name + ".xml";
	
	bool res = xml.SaveToFile(outFile);
	if ( !res ) {
		errorMessage_ = "Unable to save file " + outFile;
		return false;
	}

	ServerSettingsStruct &ss = serversSettings_[newName];
	ss.setParam("hostname",serverName);
	ss.setParam("folder",remoteDirectory);
	ss.setParam("downloadPath",downloadUrl);
	ss.authData.Login = login;
	ss.authData.Password = password;
	ss.authData.DoAuth = !login.empty();
	createdServerName_ = newName;
	return uploadEngineList_->LoadFromFile(outFile,serversSettings_);
}

std::string ServerListManager::errorMessage() const
{
	return errorMessage_;
}

std::string ServerListManager::createdServerName() const
{
	return createdServerName_;
}
