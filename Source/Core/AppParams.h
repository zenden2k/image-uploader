#ifndef IU_CORE_APPPARAMS_H
#define IU_CORE_APPPARAMS_H

#include "Core/Utils/Singleton.h"
#include <string>

class AppParams: public Singleton<AppParams>
{
    public:
        std::string dataDirectory();
        void setDataDirectory(const std::string& directory);
        std::string settingsDirectory();
        void setSettingsDirectory(const std::string& directory);
        std::string languageFile();
        void setLanguageFile(const std::string& languageFile);
        void setTempDirectory(const std::string& directory);
        std::string tempDirectory() const;
    protected:
        std::string dataDirectory_;
        std::string settingsDirectory_;
        std::string languageFile_;
        std::string tempDirectory_;
};

#endif