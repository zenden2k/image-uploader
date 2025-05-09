#include "VideoUtils.h"

#include "Core/Utils/StringUtils.h"
#include "Core/CommonDefs.h"

VideoUtils& VideoUtils::instance()
{
    static VideoUtils theSingleInstance;
    return theSingleInstance;
} 

VideoUtils::VideoUtils(){
    std::vector<std::string> extensions;

    IuStringUtils::Split(IU_VIDEOFILES_EXTENSIONS, IU_EXTENSIONS_LIST_SEPARATOR, extensions, -1);
    videoFilesExtensionsSet.insert(extensions.begin(), extensions.end());
    extensions.clear();

    IuStringUtils::Split(IU_AUDIOFILES_EXTENSIONS, IU_EXTENSIONS_LIST_SEPARATOR, extensions, -1);
    audioFilesExtensionsSet.insert(extensions.begin(), extensions.end());

}
