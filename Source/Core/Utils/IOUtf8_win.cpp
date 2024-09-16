extern "C" {
#include "IOUtf8_win.h"
}

#include "Core/Utils/CoreUtils.h"

FILE* fopen_utf8(char const* fileName, char const* mode)
{
#ifdef _MSC_VER
    return _wfopen(IuCoreUtils::Utf8ToWstring(fileName).c_str(), IuCoreUtils::Utf8ToWstring(mode).c_str());
#else
    return fopen(IuCoreUtils::Utf8ToSystemLocale(filename).c_str(), mode);
#endif
}

int stat_utf8(char const* const _FileName, struct stat* const _Stat)
{
#ifdef _MSC_VER
    #ifdef _USE_32BIT_TIME_T
        _STATIC_ASSERT(sizeof(struct stat) == sizeof(struct _stat32));
        return _wstat32(_FileName, (struct _stat32*)_Stat);
    #else
    _STATIC_ASSERT(sizeof(struct stat) == sizeof(struct _stat64i32));
    return _wstat64i32(IuCoreUtils::Utf8ToWstring(_FileName).c_str(), (struct _stat64i32*)_Stat);
    #endif
    //return _wstat(IuCoreUtils::Utf8ToWstring(_FileName).c_str(), _Stat);
#else
    return stat(IuCoreUtils::Utf8ToSystemLocale(_FileName).c_str(), _Stat);
#endif
}
