#include "logging.h"
#include <Core/Utils/CoreUtils.h>

std::ostream& operator<<(std::ostream& out, const wchar_t* str) {
	out << IuCoreUtils::WstringToUtf8(str);
	return out;
}

std::ostream& operator<<(std::ostream& out, const std::wstring& str) {
	std::string msg = IuCoreUtils::WstringToUtf8(str).c_str();
	
	return operator<<(out, msg);
}