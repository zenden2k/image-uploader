#include "ServerProfile.h"
#include "Core/SettingsManager.h"
#include "Core/Settings.h"
#include "Core/ServiceLocator.h"

ServerProfile::ServerProfile() {
    UseDefaultSettings = true;
    shortenLinks_ = false;
}

ServerProfile::ServerProfile(const std::string&  newServerName){
    serverName_ = newServerName;
    UseDefaultSettings = true;
    shortenLinks_ = false;
}

void ServerProfile::setProfileName(const std::string& newProfileName) {
    profileName_ = newProfileName;
    UseDefaultSettings = true;
}

std::string ServerProfile::profileName() const {
    return profileName_;
}

void ServerProfile::setServerName(const std::string& newServerName){
    serverName_ = newServerName;
}

std::string ServerProfile::serverName() const {
    return serverName_;
}

std::string ServerProfile::folderTitle() const
{
    return folderTitle_;
}

void ServerProfile::setFolderTitle(const std::string& newTitle)
{
    folderTitle_ = newTitle;
}

std::string ServerProfile::folderId() const
{
    return folderId_;
}

void ServerProfile::setFolderId(const std::string& newId)
{
    folderId_ = newId;
}

std::string ServerProfile::folderUrl() const
{
    return folderUrl_;
}

void ServerProfile::setFolderUrl(const std::string& newUrl)
{
    folderUrl_ = newUrl;
}

bool ServerProfile::shortenLinks() const
{
    return shortenLinks_;
}

void ServerProfile::setShortenLinks(bool shorten)
{
    shortenLinks_ = shorten;
}

bool ServerProfile::isNull()
{
    return serverName_.empty();
}

void ServerProfile::clearFolderInfo()
{
    folderUrl_.clear();
    folderTitle_.clear();
    folderId_.clear();
}

ServerProfile ServerProfile::deepCopy()
{
    ServerProfile res = *this;
#ifdef IU_WTL
    res.imageUploadParams = getImageUploadParams();
#endif
    res.UseDefaultSettings = false;
    UseDefaultSettings = false;
    return res;
}

#ifndef IU_SERVERLISTTOOL
void ServerProfile::bind(SettingsNode& serverNode)
{
    serverNode["@Name"].bind(serverName_);
    serverNode["@FolderId"].bind(folderId_);
    //MessageBoxA(0,folderTitle_.c_str(),0,0);
    serverNode["@FolderTitle"].bind(folderTitle_);
    serverNode["@FolderUrl"].bind(folderUrl_);
    serverNode["@ProfileName"].bind(profileName_);
    serverNode["@UseDefaultSettings"].bind(UseDefaultSettings);
    serverNode["@ShortenLinks"].bind(shortenLinks_);
#ifdef IU_WTL
    imageUploadParams.bind(serverNode);
#endif
}
#endif

#ifdef IU_WTL
ImageUploadParams ServerProfile::getImageUploadParams()
{
#ifndef IU_SERVERLISTTOOL
    if (UseDefaultSettings && &Settings.imageServer != this) {
        return Settings.imageServer.imageUploadParams;
    }
#endif
    return imageUploadParams;
}

ImageUploadParams& ServerProfile::getImageUploadParamsRef()
{
    return imageUploadParams;
}

void ServerProfile::setImageUploadParams(ImageUploadParams iup)
{
    imageUploadParams = iup;
}
#endif

ServerSettingsStruct& ServerProfile::serverSettings() {
    ServerSettingsStruct* res = Settings.getServerSettings(*this);
    res->setParam("FolderID", folderId_);
    res->setParam("FolderUrl", folderUrl_);
    res->setParam("FolderTitle", folderTitle_);
    return *res;
}

CUploadEngineData* ServerProfile::uploadEngineData() const {
    return ServiceLocator::instance()->engineList()->byName(serverName_);
}
