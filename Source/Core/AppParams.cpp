#include "AppParams.h"

AppParams::AppParams() {
}

std::string AppParams::dataDirectory()
{
    return dataDirectory_;
}

void AppParams::setDataDirectory(const std::string& directory)
{
    settingsDirectory_ = directory;
}

std::string AppParams::settingsDirectory()
{    
    return settingsDirectory_;
}

void AppParams::setSettingsDirectory(const std::string& directory)
{
    settingsDirectory_ = directory;
}

std::string AppParams::languageFile()
{
    return languageFile_;
}

void AppParams::setLanguageFile(const std::string& languageFile)
{
    languageFile_ = languageFile;
}

void AppParams::setTempDirectory(const std::string& directory) {
    tempDirectory_ = directory;
}

std::string AppParams::tempDirectory() const {
    return tempDirectory_;
}