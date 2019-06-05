#include "VideoUtils.h"

#include "Core/Utils/StringUtils.h"
#include "Core/CommonDefs.h"

VideoUtils& VideoUtils::Instance()
{
    static VideoUtils theSingleInstance;
    return theSingleInstance;
} 

VideoUtils::VideoUtils(){
    std::string formats{ IU_VIDEOFILES_EXTENSIONS };

    IuStringUtils::Split(formats, IU_VIDEOFILES_EXTENSIONS_SEPARATOR, videoFilesExtensions, -1);
}