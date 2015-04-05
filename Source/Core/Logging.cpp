#include "logging.h"
#include <Core/Utils/CoreUtils.h>

std::ostream& operator<<(std::ostream& out, const wchar_t* str) {
	std::string msg = IuCoreUtils::WstringToUtf8(str);
	out << msg;
	return out; 
}

std::ostream& operator<<(std::ostream& out, const std::wstring& str) {
	std::string msg = IuCoreUtils::WstringToUtf8(str).c_str();
	
	return operator<<(out, msg);
}

#ifdef _WIN32
std::ostream& operator<<(std::ostream& out, RECT rc) {
	char buffer[100];
	sprintf(buffer, "%d %d %d %d", rc.top, rc.left, rc.right - rc.left, rc.bottom - rc.top);
	return operator<<(out, buffer); ;
}

#endif