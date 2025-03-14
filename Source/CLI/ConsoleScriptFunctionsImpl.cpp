
#include "Core/Scripting/API/ScriptFunctionsImpl.h"

#include "Core/Utils/StringUtils.h"

namespace ScriptAPI::Impl {

std::string GetAppLanguageImpl() {
    return "en";
}

std::string GetAppLocaleImpl() {
    return "en_US";
}

}
