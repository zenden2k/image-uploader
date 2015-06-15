#ifndef IU_CORE_UTILS_SYSTEMUTILS_WIN_H
#define IU_CORE_UTILS_SYSTEMUTILS_WIN_H

#include <string>
#include <windows.h>
namespace IuCoreUtils
{

bool isOs64Bit() {
#if _WIN64
        return true;

#elif _WIN32

        BOOL isWow64 = FALSE;

        //IsWow64Process is not available on all supported versions of Windows.
        //Use GetModuleHandle to get a handle to the DLL that contains the function
        //and GetProcAddress to get a pointer to the function if available.
        typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
        LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)
            GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

        if (fnIsWow64Process) {
            if (!fnIsWow64Process(GetCurrentProcess(), &isWow64))
                return false;

            if (isWow64)
                return true;
            else
                return false;
        } else
            return false;
#else
        assert(0);
        return false;
#endif
    }

std::string getOsName() {
    return "Windows";
}

std::string getOsVersion() {
    OSVERSIONINFOEX osvi;

    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&osvi));

    char res[40];
    sprintf(res, "%u.%u.%u SP %u.%u", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber, 
        osvi.wServicePackMajor, osvi.wServicePackMinor);
    
    return res;
}



}

#endif