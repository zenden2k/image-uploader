#ifndef IU_CORE_I18N_TRANSLATOR_H
#define IU_CORE_I18N_TRANSLATOR_H

#pragma once
#include <string>
#include "Core/ServiceLocator.h"
#include "Core/Utils/CoreUtils.h"
#include <boost/locale.hpp>
//#include <libintl.h>

// Begin: translation macros
//#define tr(s) ServiceLocator::instance()->translator()->translate(s)
#define tr(s) boost::locale::translate(s)
#define _(s) boost::locale::translate(s)

#ifdef _WIN32
    // TR() - macro which translates wide string literal to current language
    #ifdef IU_TESTS
    #define TR(str) _T(str)
    #else
    //#define TR(str) ServiceLocator::instance()->translator()->translateW(_T(str))
    #define TR(str) (IuCoreUtils::Utf8ToWstring(boost::locale::translate(str)).c_str())
    #endif

    // TRC() macro for translating dialog item text
    #ifdef NDEBUG
    #define TRC(c, str) SetDlgItemText(c, TR(str))
    #else
    #define TRC(c, str) (ATLASSERT(GetDlgItem(c)),SetDlgItemText(c, TR(str)), (void)0)
    #endif
    //#define TR_CONST(str) const_cast<LPWSTR>(TR(str))
#endif
// End: translation macros

class ITranslator {
public:
    virtual ~ITranslator() {};
    virtual std::string getCurrentLanguage() = 0;
    virtual std::string getCurrentLocale() = 0;
    virtual std::string translate(const char* str) = 0;
#ifdef _WIN32
    virtual const wchar_t* translateW(const wchar_t* str) = 0;
#endif
    virtual bool isRTL() const {
        return false;
    };
};
#endif