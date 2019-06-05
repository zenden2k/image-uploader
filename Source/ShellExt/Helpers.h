#ifndef IU_SHELLEXT_HELPERS_H
#define IU_SHELLEXT_HELPERS_H

#include <windows.h>
#include <atlbase.h>
#include <atlcoll.h>
#include <atlstr.h>

namespace Helpers {

bool IsVistaOrLater();
bool FileExists(LPCWSTR FileName);
LPWSTR ExtractFilePath(LPCWSTR FileName, LPWSTR buf, size_t bufferSize);
LPCWSTR GetFileExt(LPCWSTR szFileName);
bool IsImage(LPCWSTR szFileName);
bool IsDirectory(LPCWSTR szFileName);
CString GetAppFolder();
CString FindDataFolder();
bool IsVideoFile(LPCWSTR szFileName);

}
#endif