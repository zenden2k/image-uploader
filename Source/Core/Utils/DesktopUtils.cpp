#include "DesktopUtils.h"

#include "CoreUtils.h"
#ifdef _WIN32
#include <shellapi.h>
#endif

namespace DesktopUtils {

bool ShellOpenUrl(const std::string& url) {
#ifdef _WIN32
    HINSTANCE hinst = ShellExecute(0, L"open", IuCoreUtils::Utf8ToWstring(url).c_str(), NULL, NULL, SW_SHOWNORMAL);
    if ( reinterpret_cast<int>(hinst) <= 32) {
        LOG(ERROR) << "ShellExecute failed. Error code=" << reinterpret_cast<int>(hinst);
        return false;
    }
    return true;
#else
#ifdef __APPLE__
    return system(("open \"" + url + "\"").c_str());
#else
    return system(("xdg-open \"" + url + "\" >/dev/null 2>&1 & ").c_str());
#endif
#endif
}

}
