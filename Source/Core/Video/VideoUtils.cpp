#include "VideoUtils.h"

#include "Core/Utils/StringUtils.h"
#include "Core/CommonDefs.h"

VideoUtils& VideoUtils::instance()
{
    static VideoUtils theSingleInstance;
    return theSingleInstance;
} 

VideoUtils::VideoUtils(){
    IuStringUtils::Split(IU_VIDEOFILES_EXTENSIONS, IU_VIDEOFILES_EXTENSIONS_SEPARATOR, videoFilesExtensions, -1);
}

CString VideoUtils::prepareVideoDialogFilters() const {
    CString result;
    for (const auto& ex: videoFilesExtensions) {
        result += _T("*.");
        result += ex.c_str();
        result += _T(";");
    }
    return result;
}