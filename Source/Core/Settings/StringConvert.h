#ifndef IU_CORE_SETTINGS_STRINGCONVERT_H
#define IU_CORE_SETTINGS_STRINGCONVERT_H

#ifdef _WIN32
#include "EncodedPassword.h"
#include "atlheaders.h"
inline std::string myToString(const CString& value)
{
    return IuCoreUtils::WstringToUtf8((LPCTSTR)value);
}

inline void myFromString(const std::string& text, CString& value)
{
    value = IuCoreUtils::Utf8ToWstring(text).c_str();
}
#ifndef IU_CLI


#endif
#endif

template<class T> std::string myToString(const EnumWrapper<T>& value)
{
    return IuCoreUtils::toString(value.value_);
}

template<class T> void myFromString(const std::string& text, EnumWrapper<T>& value)
{
    value = static_cast<T>(atoi(text.c_str()));
}

#if !defined(IU_CLI) && !defined(IU_SHELLEXT)


inline std::string myToString(const CEncodedPassword& value) {
    return (value.toEncodedData());
}

inline void myFromString(const std::string& text, CEncodedPassword& value) {
    value.fromEncodedData((text));
}

#endif

#endif
