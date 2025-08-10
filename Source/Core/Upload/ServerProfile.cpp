#include "ServerProfile.h"

#include "Core/SettingsManager.h"
#include "Core/Settings/BasicSettings.h"
#include "Core/ServiceLocator.h"
#ifdef _WIN32
#include "Core/Settings/CommonGuiSettings.h"
#endif

ServerProfile::ServerProfile() {
    shortenLinks_ = false;
}

ServerProfile::ServerProfile(const std::string&  newServerName){
    serverName_ = newServerName;
    UseDefaultSettings = false;
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

CFolderItem ServerProfile::folder() const {
    return folder_;
}

void ServerProfile::setFolder(const CFolderItem& folder) {
    folder_ = folder;
}

std::string ServerProfile::folderTitle() const
{
    return folder_.getTitle();
}

void ServerProfile::setFolderTitle(const std::string& newTitle)
{
    folder_.setTitle(newTitle);
}

std::string ServerProfile::folderId() const
{
    return folder_.getId();
}

void ServerProfile::setFolderId(const std::string& newId)
{
    folder_.setId(newId);
}

std::string ServerProfile::folderUrl() const
{
    return folder_.getViewUrl();
}

void ServerProfile::setFolderUrl(const std::string& newUrl)
{
    folder_.setViewUrl(newUrl);
}

bool ServerProfile::shortenLinks() const
{
    return shortenLinks_;
}

void ServerProfile::setShortenLinks(bool shorten)
{
    shortenLinks_ = shorten;
}

void ServerProfile::setParentIds(const std::vector<std::string> parentIds) {
    folder_.parentIds = parentIds;
}

const std::vector<std::string>& ServerProfile::parentIds() const {
    return folder_.parentIds;
}

bool ServerProfile::isNull() const
{
    return serverName_.empty();
}

void ServerProfile::clearFolderInfo()
{
    folder_ = {};
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
    serverNode["@FolderId"].bind(folder_.id);
    serverNode["@FolderTitle"].bind(folder_.title);
    serverNode["@FolderUrl"].bind(folder_.viewUrl);
    serverNode["@ProfileName"].bind(profileName_);
    serverNode["@UseDefaultSettings"].bind(UseDefaultSettings);
    serverNode["@ShortenLinks"].bind(shortenLinks_);
    serverNode["@ParentIds"].bind(folder_.parentIds);
#ifdef _WIN32
    imageUploadParams.bind(serverNode);
#endif
}

ImageUploadParams ServerProfile::getImageUploadParams()
{
#ifdef _WIN32
    auto* settings = ServiceLocator::instance()->settings<CommonGuiSettings>();
    if (UseDefaultSettings && settings) {
        return settings->DefaultImageUploadParams;
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
