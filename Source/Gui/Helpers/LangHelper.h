#ifndef IU_GUI_HELPERS_LANGHELPER_H
#define IU_GUI_HELPERS_LANGHELPER_H

#include <map>
#include <string>
#include <unordered_map>
#include "Core/Utils/Singleton.h"

class LangHelper : public Singleton<LangHelper> {
public:
    struct SystemLocaleItem
    {
        std::wstring nativeName, englishName;
    };
    LangHelper();
    std::map<std::string, std::string> getLanguageList(const std::wstring& languagesDirectory) const;
    const std::unordered_map<std::string, std::string>& getLocaleList() const;
private:
    std::unordered_map<std::string, std::string> localeNames_;
    std::map<std::wstring, SystemLocaleItem> systemLocales_;
};

#endif
