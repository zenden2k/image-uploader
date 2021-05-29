#ifndef IU_GUI_HELPERS_LANGHELPER_H
#define IU_GUI_HELPERS_LANGHELPER_H

#include <string>
#include <vector>

namespace LangHelper {

std::vector<std::wstring> getLanguageList(const std::wstring& languagesDirectory);

}

#endif