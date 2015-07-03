#ifndef IU_CORE_I18N_TRANSLATOR_H
#define IU_CORE_I18N_TRANSLATOR_H

#pragma once
#include <string>

class ITranslator {
public:
    virtual ~ITranslator() {};
    virtual std::string getCurrentLanguage() = 0;
    virtual std::string getCurrentLocale() = 0;
};
#endif