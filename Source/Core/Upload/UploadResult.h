#ifndef IU_CORE_UPLOAD_UPLOADRESULT_H
#define IU_CORE_UPLOAD_UPLOADRESULT_H
#include "Core/Utils/CoreTypes.h"

#pragma once
#include <string>
/**
UploadResult
*/
struct UploadResult
{
public:
    /*! @cond PRIVATE */
	std::string directUrl;
	std::string directUrlShortened;
	std::string thumbUrl;
	std::string downloadUrl;
	std::string downloadUrlShortened;
	std::string serverName;
    /* @endcond */


    std::string getDirectUrl() const
    {
        return directUrl;
    }

    void setDirectUrl(const std::string& basicString)
    {
        directUrl = basicString;
    }

    std::string getDirectUrlShortened() const
    {
        return directUrlShortened;
    }

    void setDirectUrlShortened(const std::string& basicString)
    {
        directUrlShortened = basicString;
    }

    std::string getThumbUrl() const
    {
        return thumbUrl;
    }

    void setThumbUrl(const std::string& basicString)
    {
        thumbUrl = basicString;
    }

    std::string getDownloadUrl() const
    {
        return downloadUrl;
    }

    void setDownloadUrl(const std::string& basicString)
    {
        downloadUrl = basicString;
    }

    std::string getDownloadUrlShortened() const
    {
        return downloadUrlShortened;
    }

    void setDownloadUrlShortened(const std::string& basicString)
    {
        downloadUrlShortened = basicString;
    }

    std::string getServerName() const
    {
        return serverName;
    }

    void setServerName(const std::string& basicString)
    {
        serverName = basicString;
    }

    UploadResult()
	{
	}
};

#endif
