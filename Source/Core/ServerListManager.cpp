/*

Image Uploader -  free application for uploading images/files to the Internet

Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "ServerListManager.h"

#include "Core/Utils/SimpleXml.h"
#include "UploadEngineList.h"
#include "Core/Utils/StringUtils.h"

ServerListManager::ServerListManager(const std::string &serversDirectory, CUploadEngineList* uel, ServerSettingsMap& serversSettings): 
    serversSettings_(serversSettings)
{
    uploadEngineList_ = uel;
    serversDirectory_ = serversDirectory;
}


std::string ServerListManager::addFtpServer(ServerType serverType, const std::string &name, const std::string &serverName, const std::string &login, const std::string &password, const std::string &remoteDirectory, const std::string &downloadUrl, 
    const std::string& privateKeyFile)
{
    SimpleXml xml;
    SimpleXmlNode root = xml.getRoot("Servers");
    std::string newName = name + (serverType == ServerType::stSFTP ? " (sftp)" : " (ftp)");

    if ( uploadEngineList_->byName(newName) ) {
        throw std::runtime_error("Server with such name already exists.");
    }

    SimpleXmlNode serverNode = root.GetChild("Server");
    serverNode.SetAttribute("Name", newName);
    serverNode.SetAttribute("Plugin", serverType == ServerType::stSFTP ? "sftp" : "ftp");
    serverNode.SetAttribute("FileHost", 1);
    serverNode.SetAttribute("Authorize", 1);

    SimpleXmlNode resultNode = serverNode.GetChild("Result");
    resultNode.SetAttribute("ImageUrlTemplate", "stub");
    resultNode.SetAttribute("ThumbUrlTemplate", "stub");
    resultNode.SetAttribute("DownloadUrlTemplate", "stub");

    const std::string outFile = serversDirectory_ + name + ".xml";
    if ( !IuCoreUtils::DirectoryExists(serversDirectory_)) {
        if (!IuCoreUtils::createDirectory(serversDirectory_)) {
            throw std::runtime_error("Cannot create directory " + serversDirectory_);
        }
    }
    
    const bool res = xml.SaveToFile(outFile);
    if ( !res ) {
        throw std::runtime_error("Unable to save file " + outFile);
    }

    ServerSettingsStruct &ss = serversSettings_[newName][login];
    ss.setParam("hostname",serverName);
    ss.setParam("folder",remoteDirectory);
    ss.setParam("downloadPath",downloadUrl);

    if (serverType == ServerType::stSFTP) {
        ss.setParam("privateKeyPath", privateKeyFile);
    }
    ss.authData.Login = login;
    ss.authData.Password = password;
    ss.authData.DoAuth = !login.empty();
    if (!uploadEngineList_->loadFromFile(outFile, serversSettings_)) {
        throw std::runtime_error("Unable to load file " + outFile);
    }
    return newName;
}

std::string ServerListManager::addDirectoryAsServer(const std::string &name, const std::string &directory, const std::string &downloadUrl, bool convertUncPath)
{
    SimpleXml xml;
    SimpleXmlNode root = xml.getRoot("Servers");

    if ( uploadEngineList_->byName(name) ) {
        throw std::runtime_error("Server with such name already exists.");
    }

    SimpleXmlNode serverNode = root.GetChild("Server");
    serverNode.SetAttribute("Name", name);
    serverNode.SetAttribute("Plugin", "directory");
    serverNode.SetAttribute("FileHost", 1);
    serverNode.SetAttribute("Authorize", 0);

    SimpleXmlNode resultNode = serverNode.GetChild("Result");
    resultNode.SetAttribute("ImageUrlTemplate", "stub");
    resultNode.SetAttribute("ThumbUrlTemplate", "stub");
    resultNode.SetAttribute("DownloadUrlTemplate", "stub");

    std::string filename = IuStringUtils::Replace(name,":","_");
    filename = IuStringUtils::Replace(filename,"\\","_");
    filename = IuStringUtils::Replace(filename," ","_");
    filename = IuStringUtils::Replace(filename,"/","_");
    const std::string outFile = serversDirectory_ + filename + ".xml";
    if ( !IuCoreUtils::DirectoryExists(serversDirectory_)) {
        if (!IuCoreUtils::createDirectory(serversDirectory_)) {
            throw std::runtime_error("Cannot create directory " + serversDirectory_);
        }
    }

    const bool res = xml.SaveToFile(outFile);
    if ( !res ) {
        throw std::runtime_error("Unable to save file " + outFile);
    }

    ServerSettingsStruct &ss = serversSettings_[name][""];
    ss.setParam("directory",directory);
    ss.setParam("downloadUrl",downloadUrl);
    ss.setParam("convertUncPath",std::to_string(static_cast<int>(convertUncPath)));
    ss.authData.DoAuth = false;

   
    if (!uploadEngineList_->loadFromFile(outFile,serversSettings_)) {
        throw std::runtime_error("Unable to load file " + outFile);
    }
    return name;
}
