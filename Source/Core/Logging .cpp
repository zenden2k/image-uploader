#include "logging.h"
#include <Core/Utils/CoreUtils.h>

std::ostream& operator<<(std::ostream& out, const wchar_t* str) {
	out << IuCoreUtils::WstringToUtf8(str);
	return out;
}

std::ostream& operator<<(std::ostream& out, const std::wstring& str) {
	return operator<<(out, str.c_str());
}