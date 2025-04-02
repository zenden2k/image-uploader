#ifndef IU_SHELLEXT_HELPERS_H
#define IU_SHELLEXT_HELPERS_H

#include <string>
#include <unordered_set>

#include <windows.h>
#include <atlbase.h>
#include <atlcoll.h>
#include <atlstr.h>

namespace Helpers {

class VideoUtils {
public:
    static VideoUtils& instance();
    std::unordered_set<std::string> videoFilesExtensionsSet;
    std::unordered_set<std::string> audioFilesExtensionsSet;
    VideoUtils(const VideoUtils&) = delete;
    VideoUtils& operator=(const VideoUtils&) = delete;

private:
    VideoUtils();
};

bool IsVistaOrLater();
bool FileExists(LPCWSTR FileName);
LPWSTR ExtractFilePath(LPCWSTR FileName, LPWSTR buf, size_t bufferSize);
LPCWSTR GetFileExt(LPCWSTR szFileName);
bool IsImage(LPCWSTR szFileName);
bool IsDirectory(LPCWSTR szFileName);
CString GetAppFolder();
CString FindDataFolder();

bool IsFileOfType(LPCWSTR szFileName, const std::unordered_set<std::string>& extensionsSet);

inline bool IsVideoFile(LPCWSTR szFileName) {
    return IsFileOfType(szFileName, VideoUtils::instance().videoFilesExtensionsSet);
}

inline bool IsAudioFile(LPCWSTR szFileName)
{
    return IsFileOfType(szFileName, VideoUtils::instance().audioFilesExtensionsSet);
}

void Split(const std::string& str, const std::string& delimiters, std::vector<std::string>& tokens, int maxCount);

}
#endif
