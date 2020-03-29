#ifndef CORE_LOGGING_H
#define CORE_LOGGING_H

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <cwchar>

#include <iosfwd>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif

std::ostream& operator<<(std::ostream& out, const wchar_t* str);

std::ostream& operator<<(std::ostream& out, const std::wstring& str);

#ifdef _WIN32
std::ostream& operator<<(std::ostream& out, const RECT& rc);

#endif

#endif