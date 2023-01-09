#ifndef IU_GUI_HELPERS_LANGHELPER_H
#define IU_GUI_HELPERS_LANGHELPER_H

#include <map>
#include <string>
#include <unordered_map>

namespace LangHelper {

std::map<std::string, std::string> getLanguageList(const std::wstring& languagesDirectory);
std::unordered_map<std::string, std::string>& getLocaleList();
}

#endif