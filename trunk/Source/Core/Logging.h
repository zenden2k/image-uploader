#ifndef WCHAR_LOGGING_H_
#define WCHAR_LOGGING_H_

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <wchar.h>

#include <iostream>
#include <string>

std::ostream& operator<<(std::ostream& out, const wchar_t* str);

std::ostream& operator<<(std::ostream& out, const std::wstring& str);

#endif  // WCHAR_LOGGING_H_