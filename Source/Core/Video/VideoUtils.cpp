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