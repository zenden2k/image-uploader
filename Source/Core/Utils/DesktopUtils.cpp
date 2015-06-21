#include "DesktopUtils.h"

#include "CoreUtils.h"
#ifdef _WIN32
#include <shellapi.h>
#endif

namespace DesktopUtils {

bool ShellOpenUrl(const std::string& url) {
#ifdef _WIN32
    return ShellExecute(0, L"open", IuCoreUtils::Utf8ToWstring(url).c_str(), NULL, NULL, SW_SHOWNORMAL) != 0;
#else
#ifdef __APPLE__
    return system(("open \"" + url + "\"").c_str());
#else
    return system(("xdg-open \"" + url + "\" >/dev/null 2>&1 & ").c_str());
#endif
#endif
}

}
