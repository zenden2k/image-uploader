#include "ServerProfile.h"

#include "Core/SettingsManager.h"
#include "Core/Settings/BasicSettings.h"
#include "Core/ServiceLocator.h"
#ifdef _WIN32
#include "Core/Settings/CommonGuiSettings.h"
#endif

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

bool ServerProfile::isNull() const
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
    res.imageUploadParams = getImageUploadParams();
    res.UseDefaultSettings = false;
    UseDefaultSettings = false;
    return res;
}

void ServerProfile::bind(SettingsNode& serverNode)
{
    serverNode["@Name"].bind(serverName_);
    serverNode["@FolderId"].bind(folderId_);
    serverNode["@FolderTitle"].bind(folderTitle_);
    serverNode["@FolderUrl"].bind(folderUrl_);
    serverNode["@ProfileName"].bind(profileName_);
    serverNode["@UseDefaultSettings"].bind(UseDefaultSettings);
    serverNode["@ShortenLinks"].bind(shortenLinks_);
#ifdef _WIN32
    imageUploadParams.bind(serverNode);
#endif
}

ImageUploadParams ServerProfile::getImageUploadParams()
{
#ifdef _WIN32
    CommonGuiSettings* Settings = ServiceLocator::instance()->settings<CommonGuiSettings>();
    if (UseDefaultSettings && Settings && !Settings->imageServer.isEmpty() && &Settings->imageServer.getByIndex(0) != this) {
        return Settings->imageServer.getByIndex(0).imageUploadParams;
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
    imageUploadParams = std::move(iup);
}

// TODO: Remove this method
CUploadEngineData* ServerProfile::uploadEngineData() const {
    return ServiceLocator::instance()->engineList()->byName(serverName_);
}
