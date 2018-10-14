#ifndef IU_CORE_APPPARAMS_H
#define IU_CORE_APPPARAMS_H

#include "Core/Utils/Singleton.h"
#include <string>

class AppParams: public Singleton<AppParams>
{
    public:
        struct AppVersionInfo {
            unsigned int Major;
            unsigned int Minor;
            unsigned int Release;
            unsigned int Build;
            std::string FullVersion;
            std::string BuildDate;
            AppVersionInfo() {
                Major = 0;
                Minor = 0;
                Release = 0;
                Build = 0;
            }
        };
		AppParams();
        std::string dataDirectory();
        void setDataDirectory(const std::string& directory);
        std::string settingsDirectory();
        void setSettingsDirectory(const std::string& directory);
        std::string languageFile();
        void setLanguageFile(const std::string& languageFile);
        void setTempDirectory(const std::string& directory);
        std::string tempDirectory() const;
        AppVersionInfo const* GetAppVersion() const;
        void setIsGui(bool isGui);
        bool isGui() const;
    protected:
        std::string dataDirectory_;
        std::string settingsDirectory_;
        std::string languageFile_;
        std::string tempDirectory_;
        AppVersionInfo versionInfo_;
        bool isGui_;
};

#endif