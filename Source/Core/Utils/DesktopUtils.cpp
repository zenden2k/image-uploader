#include "DesktopUtils.h"

#include "CoreUtils.h"
#ifdef _WIN32
#include "Func/WinUtils.h"
#endif

namespace DesktopUtils {

bool ShellOpenUrl(const std::string& url) {
#ifdef _WIN32
    return WinUtils::ShellOpenFileOrUrl(U2W(url));
#else
#ifdef __APPLE__
    return system(("open \"" + url + "\"").c_str());
#else
    return system(("xdg-open \"" + url + "\" >/dev/null 2>&1 & ").c_str());
#endif
#endif
}

}
