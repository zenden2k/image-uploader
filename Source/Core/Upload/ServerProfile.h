#ifndef IU_CORE_UPLOAD_SERVERPROFILE_H
#define IU_CORE_UPLOAD_SERVERPROFILE_H

#pragma once
#include "Core/Images/ImageConverter.h"

class CUploadEngineData;
class ServerSettingsStruct;
class SettingsNode;

struct ImageUploadParams {
    ImageUploadParams() {
        UseServerThumbs = false;
        CreateThumbs = false;
        ProcessImages = false;
        UseDefaultThumbSettings = true;
        ThumbAddImageSize = true;
        ImageProfileName = "Default";
        Thumb.Size = ThumbCreatingParams::DEFAULT_THUMB_WIDTH;
        Thumb.ResizeMode = ThumbCreatingParams::trByWidth;
        Thumb.AddImageSize = true;
        Thumb.Format = ThumbCreatingParams::tfPNG;
        Thumb.TemplateName = "default";
        Thumb.BackgroundColor = 0xffffff;
        Thumb.Quality = 85;
        Thumb.Text = "%width%x%height% (%size%)";
    }
#ifdef _WIN32
    void bind(SettingsNode& n);
#endif
    bool UseServerThumbs;
    bool CreateThumbs;
    bool ProcessImages;
    bool ThumbAddImageSize;

    std::string ImageProfileName;

    bool UseDefaultThumbSettings;
    ThumbCreatingParams getThumb();
    ThumbCreatingParams& getThumbRef();
    void setThumb(const ThumbCreatingParams& tcp);
protected:
    ThumbCreatingParams Thumb;
};
class ServerProfile {

public:
    ServerProfile();
    explicit ServerProfile(const std::string& serverName);

    CUploadEngineData* uploadEngineData() const;

    void setProfileName(const std::string& newProfileName);
    std::string profileName() const;

    void setServerName(const std::string& newServerName);
    std::string serverName() const;

    std::string folderTitle() const;
    void setFolderTitle(const std::string& newTitle);
    std::string folderId() const;
    void setFolderId(const std::string& newId);
    std::string folderUrl() const;
    void setFolderUrl(const std::string& newUrl);

    bool shortenLinks() const;
    void setShortenLinks(bool shorten);

    void setParentIds(const std::vector<std::string> parentIds);
    const std::vector<std::string>& parentIds() const;
    bool isNull() const;
    bool UseDefaultSettings;
    void clearFolderInfo();
    ServerProfile deepCopy();
    void bind(SettingsNode& n);

    ImageUploadParams getImageUploadParams();
    ImageUploadParams& getImageUploadParamsRef();

    void setImageUploadParams(ImageUploadParams iup);
    friend struct ImageUploadParams;

protected:
    std::string serverName_;
    std::string profileName_;
    ImageUploadParams imageUploadParams;
    std::string folderTitle_;
    std::string folderId_;
    std::string folderUrl_;
    bool shortenLinks_;
    std::vector<std::string> parentIds_;
    friend class CommonGuiSettings;
};

#endif
