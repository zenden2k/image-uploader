#include "Core/Scripting/API/ScriptFunctionsImpl.h"

#include "Core/i18n/Translator.h"
#include "Core/Utils/CoreUtils.h"

namespace ScriptAPI::Impl {

std::string GetAppLanguageImpl() {
    ITranslator* translator = ServiceLocator::instance()->translator();
    if (!translator) {
        LOG(ERROR) << "No translator set";
    }
    else {
        return translator->getCurrentLanguage();
    }
    return "en";
}

std::string GetAppLocaleImpl() {
    ITranslator* translator = ServiceLocator::instance()->translator();
    if (!translator) {
        LOG(ERROR) << "No translator set";
    }
    else {
        return ServiceLocator::instance()->translator()->getCurrentLocale();
    }
 
    return "en_US";
}

}
