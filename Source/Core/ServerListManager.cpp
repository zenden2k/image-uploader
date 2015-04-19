
#include "ServerListManager.h"
#include "Core/Utils/SimpleXml.h"
#include "UploadEngineList.h"
#include "Core/Utils/StringUtils.h"
#include "Core/Logging.h"

ServerListManager::ServerListManager(const std::string &serversDirectory, CUploadEngineList* uel, ServerSettingsMap& serversSettings): 
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
	SimpleXml xml;
	SimpleXmlNode root = xml.getRoot("Servers");
	std::string newName = name + " (ftp)";

	if ( uploadEngineList_->byName(newName) ) {
		errorMessage_ = "Server with such name already exists.";
		return false;
	}

	SimpleXmlNode serverNode = root.GetChild("Server");
	serverNode.SetAttribute("Name", newName);
	serverNode.SetAttribute("Plugin", "ftp");
	serverNode.SetAttribute("FileHost", 1);
	serverNode.SetAttribute("Authorize", 1);

	SimpleXmlNode resultNode = serverNode.GetChild("Result");
	resultNode.SetAttribute("ImageUrlTemplate", "stub");
	resultNode.SetAttribute("ThumbUrlTemplate", "stub");
	resultNode.SetAttribute("DownloadUrlTemplate", "stub");

	std::string outFile = serversDirectory_ + name + ".xml";
	if ( !IuCoreUtils::DirectoryExists(serversDirectory_)) {
		if (!IuCoreUtils::createDirectory(serversDirectory_)) {
			LOG(ERROR) << "Cannot create directory " << serversDirectory_;
			return false;
		}
	}
	
	bool res = xml.SaveToFile(outFile);
	if ( !res ) {
		errorMessage_ = "Unable to save file " + outFile;
		return false;
	}

	ServerSettingsStruct &ss = serversSettings_[newName][login];
	ss.setParam("hostname",serverName);
	ss.setParam("folder",remoteDirectory);
	ss.setParam("downloadPath",downloadUrl);
	ss.authData.Login = login;
	ss.authData.Password = password;
	ss.authData.DoAuth = !login.empty();
	createdServerName_ = newName;
	return uploadEngineList_->LoadFromFile(outFile,serversSettings_);
}

bool ServerListManager::addDirectoryAsServer(const std::string &name, const std::string &directory, const std::string &downloadUrl, bool convertUncPath)
{
	SimpleXml xml;
	SimpleXmlNode root = xml.getRoot("Servers");
	std::string newName = name;

	if ( uploadEngineList_->byName(newName) ) {
		errorMessage_ = "Server with such name already exists.";
		return false;
	}

	SimpleXmlNode serverNode = root.GetChild("Server");
	serverNode.SetAttribute("Name", newName);
	serverNode.SetAttribute("Plugin", "directory");
	serverNode.SetAttribute("FileHost", 1);
	serverNode.SetAttribute("Authorize", 0);

	SimpleXmlNode resultNode = serverNode.GetChild("Result");
	resultNode.SetAttribute("ImageUrlTemplate", "stub");
	resultNode.SetAttribute("ThumbUrlTemplate", "stub");
	resultNode.SetAttribute("DownloadUrlTemplate", "stub");

	std::string filename = name;
	filename = IuStringUtils::Replace(filename,":","_");
	filename = IuStringUtils::Replace(filename,"\\","_");
	filename = IuStringUtils::Replace(filename," ","_");
	filename = IuStringUtils::Replace(filename,"/","_");
	std::string outFile = serversDirectory_ + filename + ".xml";
	if ( !IuCoreUtils::DirectoryExists(serversDirectory_)) {
		if (!IuCoreUtils::createDirectory(serversDirectory_)) {
			LOG(ERROR) << "Cannot create directory " << serversDirectory_;
			return false;
		}
	}

	bool res = xml.SaveToFile(outFile);
	if ( !res ) {
		errorMessage_ = "Unable to save file " + outFile;
		return false;
	}

	ServerSettingsStruct &ss = serversSettings_[newName][""];
	ss.setParam("directory",directory);
	ss.setParam("downloadUrl",downloadUrl);
	ss.setParam("convertUncPath",IuCoreUtils::int64_tToString((int)convertUncPath));
	ss.authData.DoAuth = false;
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
